/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Distributed under the GNU General Public License, version 3 or later.
 */
#include "stdafx.h"
#include "buffer.h"
#include "vert.h"

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
static int PolygonClipTop;
// RECOVERY_OBJECT: v2 0x009BC030 0x009BC034 _PolygonRemap 0x00626E10
static unsigned char *PolygonRemap;
// RECOVERY_OBJECT: v2 0x009BC038 0x009BC03C _PolygonVertexCount 0x00626470
static int PolygonVertexCount;
// RECOVERY_OBJECT: v2 0x009BC03C 0x009BC040 _PolygonY 0x00626620
static int PolygonY;
// RECOVERY_OBJECT: v2 0x009BC040 0x009BC044 _PolygonPitch 0x00626620
static int PolygonPitch;
// RECOVERY_OBJECT: v2 0x009BC044 0x009BC048 _PolygonClipRight 0x00626620
static int PolygonClipRight;
// RECOVERY_OBJECT: v2 0x009BC048 0x009BC04C _PolygonClipBottom 0x00626620
static int PolygonClipBottom;

// The remaining Gouraud family also uses these physical slots. In that
// family the top/current/bottom row slots become pixel addresses, rather
// than flat coordinates. Recovering these flat users does not establish
// complete shared-state closure or authorize separate Gouraud storage.

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
        unsigned char *pixel = PolygonPixels + PolygonY * PolygonPitch + left;
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
    PolygonPixels = reinterpret_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        PolygonVertices = vertices;
        PolygonColor = color;
        PolygonVertexCount = count;
        PolygonPitch = static_cast<int>(buffer->stride_);
        PolygonClipRight = buffer->rect1_.right - 1;
        PolygonClipLeft = buffer->rect1_.left;
        PolygonClipBottom = buffer->rect1_.bottom - 1;
        PolygonClipTop = buffer->rect1_.top;
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
                PolygonY = minimum_y;
                backward.direction = -1;
                if (setup_flat_polygon_edge(&backward, minimum_vertex)) {
                    forward.direction = 1;
                    if (setup_flat_polygon_edge(&forward, minimum_vertex)
                        && PolygonY < PolygonClipBottom) {
                        do {
                            if (PolygonY >= PolygonClipTop) {
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
                            ++PolygonY;
                        } while (PolygonY < PolygonClipBottom);
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
        unsigned char *pixel = PolygonPixels + PolygonY * PolygonPitch + left;
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
    PolygonPixels = reinterpret_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        PolygonVertices = vertices;
        PolygonRemap = remap;
        PolygonVertexCount = count;
        PolygonPitch = static_cast<int>(buffer->stride_);
        PolygonClipRight = buffer->rect1_.right - 1;
        PolygonClipLeft = buffer->rect1_.left;
        PolygonClipBottom = buffer->rect1_.bottom - 1;
        PolygonClipTop = buffer->rect1_.top;
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
                PolygonY = minimum_y;
                backward.direction = -1;
                if (setup_flat_polygon_edge(&backward, minimum_vertex)) {
                    forward.direction = 1;
                    if (setup_flat_polygon_edge(&forward, minimum_vertex)
                        && PolygonY < PolygonClipBottom) {
                        do {
                            if (PolygonY >= PolygonClipTop) {
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
                            ++PolygonY;
                        } while (PolygonY < PolygonClipBottom);
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    return 0;
}
