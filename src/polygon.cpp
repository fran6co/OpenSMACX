/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Distributed under the GNU General Public License, version 3 or later.
 */
#include "stdafx.h"
#include "buffer.h"
#include "vert.h"
#include "texture.h"
#include "general.h"
#include <float.h>

// These three row slots change interpretation during Gouraud setup. A
// numeric address avoids forming pointers outside the acquired allocation
// while walking clipped rows. This union models the observed reuse, not a
// recovered historical declaration; each original slot is one DWORD.
union PolygonRowPosition {
    int coordinate;
    uintptr_t address;
};

// The flat edge record has nine signed DWORDs, in this order. The two
// directions walk the circular vertex list from its first minimum Y.
struct FlatPolygonEdge {
    // RECOVERY_MEMBER: v1 FlatPolygonEdge direction 0x0 0x00626470
    int direction;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge remaining_rows 0x4 0x00626470
    int remaining_rows;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge next_vertex 0x8 0x00626470
    int next_vertex;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge x 0xC 0x00626470
    int x;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge x_step 0x10 0x00626470
    int x_step;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge x_direction 0x14 0x00626470
    int x_direction;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge error 0x18 0x00626470
    int error;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge remainder 0x1C 0x00626470
    int remainder;
    // RECOVERY_MEMBER: v1 FlatPolygonEdge height 0x20 0x00626470
    int height;
};

// Individually shared rasterizer slots, not an inferred aggregate or a
// per-call context. Acquisition publishes even a null result; release leaves
// the other state intact. The initial image values of these slots are zero.
// Original DWORD loads/stores establish each four-byte slot independently;
// intervening Gouraud words and the neighboring log/language globals are not
// part of these declarations. The symbols below are compiler-observed.
// RECOVERY_OBJECT: v2 0x009BC008 0x009BC00C _PolygonColor 0x00626620
static int PolygonColor;
// RECOVERY_OBJECT: v2 0x009BC00C 0x009BC010 _PolygonPixels 0x00626620
static unsigned char *PolygonPixels;
// RECOVERY_OBJECT: v2 0x009BC010 0x009BC014 _PolygonBottomVertex 0x00626470
static int PolygonBottomVertex;
// RECOVERY_OBJECT: v2 0x009BC018 0x009BC01C _PolygonVertices 0x00626470
static Vert *PolygonVertices;
// RECOVERY_OBJECT: v2 0x009BC01C 0x009BC020 _PolygonClipLeft 0x00626620
static int PolygonClipLeft;
// RECOVERY_OBJECT: v2 0x009BC020 0x009BC024 _PolygonClipTop 0x00626620
static PolygonRowPosition PolygonClipTop;
// RECOVERY_OBJECT: v2 0x009BC030 0x009BC034 _PolygonRemap 0x00626E10
static unsigned char *PolygonRemap;
// RECOVERY_OBJECT: v2 0x009BC038 0x009BC03C _PolygonVertexCount 0x00626470
static int PolygonVertexCount;
// RECOVERY_OBJECT: v2 0x009BC03C 0x009BC040 _PolygonY 0x00626620
static PolygonRowPosition PolygonY;
// RECOVERY_OBJECT: v2 0x009BC040 0x009BC044 _PolygonPitch 0x00626620
static int PolygonPitch;
// RECOVERY_OBJECT: v2 0x009BC044 0x009BC048 _PolygonClipRight 0x00626620
static int PolygonClipRight;
// RECOVERY_OBJECT: v2 0x009BC048 0x009BC04C _PolygonClipBottom 0x00626620
static PolygonRowPosition PolygonClipBottom;

// Gouraud publishes these slots separately. The light step is only a WORD;
// no access in this family establishes ownership of the following two bytes.
// RECOVERY_OBJECT: v2 0x009BC014 0x009BC018 _PolygonSpanRight 0x00626A00
static int PolygonSpanRight;
// RECOVERY_OBJECT: v2 0x009BC024 0x009BC028 _PolygonLight 0x00626A00
static int PolygonLight;
// RECOVERY_OBJECT: v2 0x009BC028 0x009BC02C _PolygonSpanLeft 0x00626A00
static int PolygonSpanLeft;
// RECOVERY_OBJECT: v2 0x009BC02C 0x009BC030 _PolygonLights 0x00626900
static int *PolygonLights;
// RECOVERY_OBJECT: v2 0x009BC034 0x009BC036 _PolygonLightStep 0x00626A00
static unsigned short PolygonLightStep;

// ORIGINAL: 0x00626470 sub_626470 0x00626470-0x00626540 FILE
// RECOVERY_BINDING: v1 {"evidence": ["626470..62653F reads two stack arguments, wraps by9BC038, skips horizontal edges and initializes direction-dependent quotient/remainder stepping."], "reason": "The original cdecl edge initializer takes a pointer to nine signed DWORDs and a vertex index, traversing the shared vertex array toward the maximum-y vertex."}
// symbol    ?setup_flat_polygon_edge@@YAHPAUFlatPolygonEdge@@H@Z
int __cdecl setup_flat_polygon_edge(FlatPolygonEdge *edge, int vertex) {
    if (vertex == PolygonBottomVertex) {
        return 0;
    }
    const int direction = edge->direction;
    int next;
    do {
        next = vertex + direction;
        if (next > PolygonVertexCount - 1) {
            next = 0;
        } else if (next < 0) {
            next = PolygonVertexCount - 1;
        }
        edge->remaining_rows = PolygonVertices[next].y - PolygonVertices[vertex].y;
        if (edge->remaining_rows != 0) {
            if (edge->remaining_rows < 0) {
                return 0;
            }
            edge->next_vertex = next;
            edge->x = PolygonVertices[vertex].x;
            int delta = PolygonVertices[next].x - PolygonVertices[vertex].x;
            if (delta < 0) {
                edge->x_direction = -1;
                delta = -delta;
                edge->error = 1 - edge->remaining_rows;
                edge->x_step = -(delta / edge->remaining_rows);
            } else {
                edge->x_direction = 1;
                edge->x_step = delta / edge->remaining_rows;
                edge->error = 0;
            }
            edge->remainder = delta % edge->remaining_rows;
            edge->height = edge->remaining_rows;
            return 1;
        }
        vertex = next;
    } while (vertex != PolygonBottomVertex);
    return 0;
}

// ORIGINAL: 0x00626540 sub_626540 0x00626540-0x00626597 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["626547 decrements rows at+4;626552 calls626470 with next vertex at+8;626561..62658D advance x and strictly-positive error. Its historical seed claim remains a restoration obligation."], "reason": "The original cdecl advance function consumes the same nine-field edge and normalizes setup success to a boolean at segment transitions."}
// symbol    ?advance_flat_polygon_edge@@YAHPAUFlatPolygonEdge@@@Z
int __cdecl advance_flat_polygon_edge(FlatPolygonEdge *edge) {
    if (--edge->remaining_rows == 0) {
        return setup_flat_polygon_edge(edge, edge->next_vertex) != 0;
    }
    edge->x += edge->x_step;
    edge->error += edge->remainder;
    if (edge->error > 0) {
        edge->x += edge->x_direction;
        edge->error -= edge->height;
    }
    return 1;
}

// ORIGINAL: 0x006265A0 ?scan_out_line_poly@@YAIHH@Z 0x006265A0-0x00626613 FILE
// RECOVERY_BINDING: v1 {"evidence": ["6265A0..626613 loads9BC008 and uses the shared row, pitch and clip scalars. The original entry produces no consistent return value; the C++ writer is void."], "reason": "The standalone solid span writer clips two x endpoints against the shared polygon state and writes the low color byte through the acquired pixels."}
// symbol    ?draw_solid_polygon_span@@YAXHH@Z
void __cdecl draw_solid_polygon_span(int left, int right) {
    if (right <= PolygonClipLeft || left >= PolygonClipRight || right - left <= 0) {
        return;
    }
    if (right >= PolygonClipRight) {
        right = PolygonClipRight;
    }
    if (left < PolygonClipLeft) {
        left = PolygonClipLeft;
    }
    int count = right - left;
    if (count > 0) {
        unsigned char *pixel = PolygonPixels + PolygonY.coordinate * PolygonPitch + left;
        const unsigned char color = static_cast<unsigned char>(PolygonColor);
        do {
            *pixel++ = color;
        } while (--count != 0);
    }
}

// Geometry follows the original signed, non-overflowing arithmetic domain.
// A nonempty polygon must have a Y below 32767 and a Y above -32765 for
// the original extrema search to establish both indices. Outside that
// domain the original reuses a pointer as its minimum index or a stale
// shared maximum index; malformed-input behavior has not been recovered.
// No minimum-index-zero fallback or new clipping policy is implied here.
// ORIGINAL: 0x00626620 ?polygon@@YAHPAUBuffer@@PAUVert@@HH@Z 0x00626620-0x00626900 FILE
// RECOVERY_BINDING: v1 {"evidence": ["5DA690 calls626620;626648 acquires Buffer storage before pitch/clip publication;6268E6 releases even failed acquisition. The original excludes right-1 and bottom-1 as limits."], "reason": "Buffer::poly directly calls this cdecl solid rasterizer with a Buffer, vertex array, count and color. The original uses two nine-field edges and shared non-reentrant scratch."}
// symbol    ?polygon@@YAHPAVBuffer@@PAUVert@@HH@Z
int __cdecl polygon(Buffer *buffer, Vert *vertices, int count, int color) {
    if (vertices == 0 || buffer == 0) {
        return 16;
    }
    PolygonPixels = static_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        PolygonVertices = vertices;
        PolygonColor = color;
        PolygonVertexCount = count;
        PolygonPitch = static_cast<int>(buffer->stride_);
        PolygonClipRight = buffer->rect1_.right - 1;
        PolygonClipLeft = buffer->rect1_.left;
        PolygonClipBottom.coordinate = buffer->rect1_.bottom - 1;
        PolygonClipTop.coordinate = buffer->rect1_.top;
        int minimum_y = 32767;
        int maximum_y = -32765;
        int minimum_vertex;
        if (count > 0) {
            for (int vertex = 0; vertex < count; ++vertex) {
                if (vertices[vertex].y < minimum_y) {
                    minimum_y = vertices[vertex].y;
                    minimum_vertex = vertex;
                }
                if (vertices[vertex].y > maximum_y) {
                    maximum_y = vertices[vertex].y;
                    PolygonBottomVertex = vertex;
                }
            }
            if (minimum_y < maximum_y) {
                FlatPolygonEdge backward;
                FlatPolygonEdge forward;
                PolygonY.coordinate = minimum_y;
                backward.direction = -1;
                if (setup_flat_polygon_edge(&backward, minimum_vertex)) {
                    forward.direction = 1;
                    if (setup_flat_polygon_edge(&forward, minimum_vertex)
                        && PolygonY.coordinate < PolygonClipBottom.coordinate) {
                        do {
                            if (PolygonY.coordinate >= PolygonClipTop.coordinate) {
                                if (backward.x < forward.x) {
                                    draw_solid_polygon_span(backward.x, forward.x);
                                } else {
                                    draw_solid_polygon_span(forward.x, backward.x);
                                }
                            }
                            if (!advance_flat_polygon_edge(&backward)) {
                                break;
                            }
                            if (!advance_flat_polygon_edge(&forward)) {
                                break;
                            }
                            ++PolygonY.coordinate;
                        } while (PolygonY.coordinate < PolygonClipBottom.coordinate);
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    return 0;
}

// ORIGINAL: 0x00626D90 ?scan_out_line_poly_dest@@YAIHH@Z 0x00626D90-0x00626E0B FILE
// RECOVERY_BINDING: v1 {"evidence": ["626D90..626E0B reads9BC030 and indexes it sequentially with each destination byte. There is no meaningful return value or copied lookup table."], "reason": "The standalone destination span writer clips endpoints against the same polygon state and performs live unsigned-byte lookup-table writes."}
// symbol    ?draw_remapped_polygon_span@@YAXHH@Z
void __cdecl draw_remapped_polygon_span(int left, int right) {
    if (right <= PolygonClipLeft || left >= PolygonClipRight || right - left <= 0) {
        return;
    }
    if (right >= PolygonClipRight) {
        right = PolygonClipRight;
    }
    if (left < PolygonClipLeft) {
        left = PolygonClipLeft;
    }
    int count = right - left;
    if (count > 0) {
        unsigned char *pixel = PolygonPixels + PolygonY.coordinate * PolygonPitch + left;
        unsigned char *remap = PolygonRemap;
        do {
            // Read the table live, including when it aliases the destination.
            *pixel = remap[*pixel];
            ++pixel;
        } while (--count != 0);
    }
}

// ORIGINAL: 0x00626E10 ?polygon_dest@@YAHPAUBuffer@@PAUVert@@HPAE@Z 0x00626E10-0x00627100 FILE
// RECOVERY_BINDING: v1 {"evidence": ["BaseWin draw_farm40F38C/40F40A call626E10;626E38 acquires before publishing geometry/clip,6270E6 releases, and shared9BC030 supplies the per-pixel remap."], "reason": "The cdecl destination-remapping rasterizer shares flat edge/storage ownership with solid polygon and consumes a live caller-provided byte lookup table."}
// symbol    ?polygon_dest@@YAHPAVBuffer@@PAUVert@@HPAE@Z
int __cdecl polygon_dest(Buffer *buffer, Vert *vertices, int count, unsigned char *remap) {
    if (vertices == 0 || buffer == 0) {
        return 16;
    }
    PolygonPixels = static_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        PolygonVertices = vertices;
        PolygonRemap = remap;
        PolygonVertexCount = count;
        PolygonPitch = static_cast<int>(buffer->stride_);
        PolygonClipRight = buffer->rect1_.right - 1;
        PolygonClipLeft = buffer->rect1_.left;
        PolygonClipBottom.coordinate = buffer->rect1_.bottom - 1;
        PolygonClipTop.coordinate = buffer->rect1_.top;
        int minimum_y = 32767;
        int maximum_y = -32765;
        int minimum_vertex;
        if (count > 0) {
            for (int vertex = 0; vertex < count; ++vertex) {
                if (vertices[vertex].y < minimum_y) {
                    minimum_y = vertices[vertex].y;
                    minimum_vertex = vertex;
                }
                if (vertices[vertex].y > maximum_y) {
                    maximum_y = vertices[vertex].y;
                    PolygonBottomVertex = vertex;
                }
            }
            if (minimum_y < maximum_y) {
                FlatPolygonEdge backward;
                FlatPolygonEdge forward;
                PolygonY.coordinate = minimum_y;
                backward.direction = -1;
                if (setup_flat_polygon_edge(&backward, minimum_vertex)) {
                    forward.direction = 1;
                    if (setup_flat_polygon_edge(&forward, minimum_vertex)
                        && PolygonY.coordinate < PolygonClipBottom.coordinate) {
                        do {
                            if (PolygonY.coordinate >= PolygonClipTop.coordinate) {
                                if (backward.x < forward.x) {
                                    draw_remapped_polygon_span(backward.x, forward.x);
                                } else {
                                    draw_remapped_polygon_span(forward.x, backward.x);
                                }
                            }
                            if (!advance_flat_polygon_edge(&backward)) {
                                break;
                            }
                            if (!advance_flat_polygon_edge(&forward)) {
                                break;
                            }
                            ++PolygonY.coordinate;
                        } while (PolygonY.coordinate < PolygonClipBottom.coordinate);
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    return 0;
}

static int polygon_fixed(int value) {
    return static_cast<int>(static_cast<unsigned int>(value) << 16);
}

static int polygon_difference(int a, int b) {
    return static_cast<int>(static_cast<unsigned int>(a) - static_cast<unsigned int>(b));
}

static void polygon_reset_fpu() {
    // FNINIT resets, rather than restores, the caller's environment. VC6's
    // _fpreset selects 53-bit precision; the image resets to 64-bit instead.
    _fpreset();
    _control87(_MCW_EM | _PC_64 | _RC_NEAR | _IC_PROJECTIVE,
               _MCW_EM | _MCW_PC | _MCW_RC | _MCW_IC);
}

// The image directly divides delta by width_fixed under PC24/chop, scales
// by the exact power of two 256, then converts to a signed WORD. Every
// in-range integer boundary is representable at that precision, so truncating
// this exact rational gives the same final WORD. Conversion overflow yields
// the x87 integer-indefinite bits. This does not emulate arithmetic exception
// flags or the pending x87 stack visible inside a release callback.
static unsigned short polygon_light_step(int delta, int width_fixed) {
    const int64_t step = static_cast<int64_t>(delta) * 256 / width_fixed;
    if (step < -32768 || step > 32767) {
        return 0x8000;
    }
    return static_cast<unsigned short>(step);
}

// This setup uses the existing fifteen-word EdgeScan representation, with
// its four UV words untouched. Matching layout is not proof that the
// original polygon and texture code used this historical C++ type name.
// 0x00626900 reads direction/remaining/next at +0/+4/+8; x and its DDA
// occupy +1C/+20/+24/+28/+2C/+30, and light/step occupy +34/+38. The
// caller's two stack records are 0x3C bytes apart. Texture's declarations
// name those same fields without fabricating values for the unused UVs.
// ORIGINAL: 0x00626900 sub_626900 0x00626900-0x006269F3 FILE
// RECOVERY_BINDING: v1 {"evidence": ["626900..6269F3 uses stack edge/index arguments, wraps indices0..3 and initializes x at+1C, light at+34 and fixed_div628AD0 light step at+38. The shared type spelling is a representation choice, not proof of an original type name."], "reason": "This cdecl initializer consumes the shared four-vertex polygon and light arrays using the existing fifteen-word EdgeScan representation; it leaves the four unused interpolant words untouched."}
// symbol    ?setup_gouraud_polygon_edge@@YAHPAUEdgeScan@@H@Z
int __cdecl setup_gouraud_polygon_edge(EdgeScan *edge, int vertex) {
    if (vertex == PolygonBottomVertex) {
        return 0;
    }
    const int direction = edge->f0;
    int next;
    do {
        next = vertex + direction;
        if (next > 3) {
            next = 0;
        } else if (next < 0) {
            next = 3;
        }
        edge->counter = PolygonVertices[next].y - PolygonVertices[vertex].y;
        if (edge->counter != 0) {
            if (edge->counter < 0) {
                return 0;
            }
            const int height = polygon_fixed(edge->counter);
            edge->id = next;
            edge->accumC = PolygonVertices[vertex].x;
            int delta = PolygonVertices[next].x - PolygonVertices[vertex].x;
            edge->deltaC = delta / edge->counter;
            if (delta < 0) {
                edge->extraDeltaC = -1;
                delta = -delta;
                edge->fracAccum = 1 - edge->counter;
            } else {
                edge->extraDeltaC = 1;
                edge->fracAccum = 0;
            }
            edge->fracDelta = delta % edge->counter;
            edge->fracSub = edge->counter;
            edge->accumD = polygon_fixed(PolygonLights[vertex]);
            edge->deltaD = fixed_div(
                polygon_difference(polygon_fixed(PolygonLights[next]), edge->accumD),
                height);
            return 1;
        }
        vertex = next;
    } while (vertex != PolygonBottomVertex);
    return 0;
}

// Four vertices, traversed in their supplied order: unlike the flat entries,
// the Gouraud writer does not sort each row's two X endpoints. Geometry has
// the extrema/fitting-division requirements documented above; acquired row
// addresses must be nonwrapping and preserve signed address order. Integer
// row addresses allow clipped negative rows without invalid pointer formation.
// Shared slots, FPU control changes and release ordering are preserved, but
// intermediate x87 status flags and stack contents are not established.
// ORIGINAL: 0x00626A00 sub_626a00 0x00626A00-0x00626D8B FILE
// RECOVERY_BINDING: v1 {"evidence": ["626A27 acquires Buffer pixels;626AED/626AFA/626B0A convert row/bottom/top storage;626B76..626B7C divide directly under PC24/chop;626C62 publishes WORD light step;626D07 shifts clipped unsigned-step product right8;626D41..626D48 write inclusive endpoints;626D6F release precedes626D75 FPU reset."], "reason": "This free cdecl rasterizer consumes exactly four Vert coordinates and four signed light values, sharing polygon scratch and converting its three row slots from coordinates to addresses."}
// symbol    ?polygon_gouraud@@YAHPAVBuffer@@PAUVert@@PAH@Z
int __cdecl polygon_gouraud(Buffer *buffer, Vert *vertices, int *lights) {
    if (vertices == 0 || lights == 0) {
        return 16;
    }
    PolygonPixels = static_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        polygon_reset_fpu();
        PolygonVertices = vertices;
        PolygonLights = lights;
        PolygonPitch = static_cast<int>(buffer->stride_);
        PolygonClipRight = buffer->rect1_.right;
        PolygonClipLeft = buffer->rect1_.left;
        PolygonClipBottom.coordinate = buffer->rect1_.bottom;
        PolygonClipTop.coordinate = buffer->rect1_.top;
        int minimum_y = 32767;
        int maximum_y = -32765;
        int minimum_vertex;
        for (int vertex = 0; vertex < 4; ++vertex) {
            if (vertices[vertex].y < minimum_y) {
                minimum_y = vertices[vertex].y;
                minimum_vertex = vertex;
            }
            if (vertices[vertex].y > maximum_y) {
                maximum_y = vertices[vertex].y;
                PolygonBottomVertex = vertex;
            }
        }
        if (minimum_y < maximum_y) {
            const uintptr_t pixels = reinterpret_cast<uintptr_t>(PolygonPixels);
            PolygonY.address = pixels + static_cast<int64_t>(minimum_y) * PolygonPitch;
            PolygonClipBottom.address = pixels
                + static_cast<int64_t>(PolygonClipBottom.coordinate) * PolygonPitch;
            PolygonClipTop.address = pixels
                + static_cast<int64_t>(PolygonClipTop.coordinate) * PolygonPitch;
            EdgeScan backward;
            EdgeScan forward;
            backward.f0 = -1;
            if (setup_gouraud_polygon_edge(&backward, minimum_vertex)) {
                forward.f0 = 1;
                if (setup_gouraud_polygon_edge(&forward, minimum_vertex)) {
                    _control87(_PC_24 | _RC_CHOP, _MCW_PC | _MCW_RC);
                    int pending_width = polygon_fixed(
                        polygon_difference(forward.accumC, backward.accumC));
                    int pending_delta = 0;
                    if (pending_width > 0) {
                        pending_delta = polygon_difference(forward.accumD, backward.accumD);
                    }
                    if (static_cast<intptr_t>(PolygonY.address)
                        < static_cast<intptr_t>(PolygonClipBottom.address)) {
                        bool done = false;
                        do {
                            PolygonSpanRight = forward.accumC;
                            PolygonSpanLeft = backward.accumC;
                            PolygonLight = backward.accumD;

                            if (--backward.counter == 0) {
                                if (!setup_gouraud_polygon_edge(&backward, backward.id)) {
                                    done = true;
                                }
                            } else {
                                backward.accumD = static_cast<int>(
                                    static_cast<unsigned int>(backward.accumD) + backward.deltaD);
                                backward.accumC += backward.deltaC;
                                backward.fracAccum += backward.fracDelta;
                                if (backward.fracAccum > 0) {
                                    backward.accumC += backward.extraDeltaC;
                                    backward.fracAccum -= backward.fracSub;
                                }
                            }
                            if (!done) {
                                if (--forward.counter == 0) {
                                    if (!setup_gouraud_polygon_edge(&forward, forward.id)) {
                                        done = true;
                                    }
                                } else {
                                    forward.accumC += forward.deltaC;
                                    forward.accumD = static_cast<int>(
                                        static_cast<unsigned int>(forward.accumD) + forward.deltaD);
                                    forward.fracAccum += forward.fracDelta;
                                    if (forward.fracAccum > 0) {
                                        forward.accumC += forward.extraDeltaC;
                                        forward.fracAccum -= forward.fracSub;
                                    }
                                }
                            }

                            if (pending_width > 0) {
                                PolygonLightStep = polygon_light_step(pending_delta, pending_width);
                            }
                            // The image prepares the next ratio even after an
                            // edge has ended, before drawing the saved span.
                            pending_width = polygon_fixed(
                                polygon_difference(forward.accumC, backward.accumC));
                            if (pending_width > 0) {
                                pending_delta = polygon_difference(forward.accumD, backward.accumD);
                            }
                            if (static_cast<intptr_t>(PolygonY.address)
                                >= static_cast<intptr_t>(PolygonClipBottom.address)) {
                                break;
                            }
                            if (static_cast<intptr_t>(PolygonY.address)
                                    >= static_cast<intptr_t>(PolygonClipTop.address)
                                && PolygonSpanRight > PolygonClipLeft
                                && PolygonSpanLeft < PolygonClipRight
                                && PolygonSpanRight - PolygonSpanLeft > 0) {
                                int right = PolygonSpanRight;
                                if (right >= PolygonClipRight) {
                                    right = PolygonClipRight;
                                }
                                if (PolygonSpanLeft < PolygonClipLeft) {
                                    const int skip = PolygonClipLeft - PolygonSpanLeft;
                                    const uint64_t product = static_cast<uint64_t>(PolygonLightStep) * skip;
                                    PolygonLight = static_cast<int>(
                                        static_cast<unsigned int>(PolygonLight)
                                        + static_cast<unsigned int>(product >> 8));
                                    PolygonSpanLeft = PolygonClipLeft;
                                }
                                int remaining = right - PolygonSpanLeft;
                                unsigned char *pixel = reinterpret_cast<unsigned char *>(
                                    PolygonY.address + PolygonSpanLeft);
                                unsigned short light = static_cast<unsigned short>(
                                    static_cast<unsigned int>(PolygonLight) >> 8);
                                const unsigned short step = PolygonLightStep;
                                do {
                                    *pixel++ = static_cast<unsigned char>(light >> 8);
                                    light = static_cast<unsigned short>(light + step);
                                } while (--remaining >= 0);
                            }
                            if (done) {
                                break;
                            }
                            PolygonY.address += PolygonPitch;
                        } while (true);
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    polygon_reset_fpu();
    return 0;
}
