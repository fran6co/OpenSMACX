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

// ORIGINAL: 0x00626470 sub_626470 0x00626470-0x00626540 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["626470..62653F reads two stack arguments, wraps by9BC038, skips horizontal edges and initializes direction-dependent quotient/remainder stepping."], "reason": "The original cdecl edge initializer takes a pointer to nine signed DWORDs and a vertex index, traversing the shared vertex array toward the maximum-y vertex."}
// symbol    ?setup_flat_polygon_edge@@YAHPAUFlatPolygonEdge@@H@Z
int __cdecl setup_flat_polygon_edge(FlatPolygonEdge *edge, int vertex) {
    int next;
    for (;;) {
        if (vertex == PolygonBottomVertex) {
            return 0;
        }
        next = vertex + edge->direction;
        if (next > PolygonVertexCount - 1) {
            next = 0;
        } else if (next < 0) {
            next = PolygonVertexCount - 1;
        }
        edge->remaining_rows = PolygonVertices[next].y - PolygonVertices[vertex].y;
        if (edge->remaining_rows != 0) {
            break;
        }
        vertex = next;
    }
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
        edge->error = 0;
        edge->x_step = delta / edge->remaining_rows;
    }
    edge->remainder = delta % edge->remaining_rows;
    edge->height = edge->remaining_rows;
    return 1;
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

// ORIGINAL: 0x006265A0 ?scan_out_line_poly@@YAIHH@Z 0x006265A0-0x00626613 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["6265A0..626613 loads9BC008 and uses the shared row, pitch and clip scalars. The original entry produces no consistent return value; the C++ writer is void."], "reason": "The standalone solid span writer clips two x endpoints against the shared polygon state and writes the low color byte through the acquired pixels."}
// symbol    ?draw_solid_polygon_span@@YAXHH@Z
// Hand-written assembly: explicit push edi/pop edi pair, one-operand imul, mov ah/[edi],ah fill.
MEASURED inline void __cdecl draw_solid_polygon_span(int left, int right) {
    __asm {
        push edi
        mov ebx, right
        cmp ebx, PolygonClipLeft
        jle span_done
        mov ecx, left
        cmp ecx, PolygonClipRight
        jge span_done
        sub ebx, ecx
        jle span_done
        mov ecx, right
        cmp ecx, PolygonClipRight
        jl right_clipped
        mov ecx, PolygonClipRight
        mov right, ecx
    right_clipped:
        mov edi, left
        cmp edi, PolygonClipLeft
        jge left_clipped
        mov edi, PolygonClipLeft
        mov left, edi
    left_clipped:
        sub ecx, edi
        jle span_done
        mov eax, PolygonY.coordinate
        imul PolygonPitch
        add eax, left
        add eax, PolygonPixels
        mov edi, eax
        mov ah, byte ptr PolygonColor
    span_fill:
        mov [edi], ah
        inc edi
        dec ecx
        jne span_fill
    span_done:
        pop edi
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
        // The image tests the clip rectangle's address before reading it.
        RECT *clip = &buffer->rect1_;
        if (clip != 0) {
            int maximum_y = -32765;
            int minimum_vertex;
            PolygonClipRight = clip->right - 1;
            PolygonClipLeft = clip->left;
            PolygonClipBottom.coordinate = clip->bottom - 1;
            PolygonClipTop.coordinate = clip->top;
            int minimum_y = 32767;
            {
                for (int vertex = 0; vertex < count; ++vertex) {
                    if (vertices[vertex].y < minimum_y) {
                        minimum_y = vertices[vertex].y;
                        minimum_vertex = vertex;
                    }
                    if (vertices[vertex].y > maximum_y) {
                        // volatile: stand-in for the image's reload of vertices[vertex].y at 0x006266D1; no plain spelling found
                        maximum_y = *(volatile int *)&vertices[vertex].y;
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
                                // Edge advance is open-coded: the image tests the
                                // raw setup result, without advance's normalization.
                                if (--backward.remaining_rows == 0) {
                                    if (!setup_flat_polygon_edge(&backward, backward.next_vertex)) {
                                        break;
                                    }
                                } else {
                                    backward.x += backward.x_step;
                                    backward.error += backward.remainder;
                                    if (backward.error > 0) {
                                        backward.x += backward.x_direction;
                                        backward.error -= backward.height;
                                    }
                                }
                                if (--forward.remaining_rows == 0) {
                                    if (!setup_flat_polygon_edge(&forward, forward.next_vertex)) {
                                        break;
                                    }
                                } else {
                                    forward.x += forward.x_step;
                                    forward.error += forward.remainder;
                                    if (forward.error > 0) {
                                        forward.x += forward.x_direction;
                                        forward.error -= forward.height;
                                    }
                                }
                                ++PolygonY.coordinate;
                            } while (PolygonY.coordinate < PolygonClipBottom.coordinate);
                        }
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    return 0;
}

// ORIGINAL: 0x00626D90 ?scan_out_line_poly_dest@@YAIHH@Z 0x00626D90-0x00626E0B FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["626D90..626E0B reads9BC030 and indexes it sequentially with each destination byte. There is no meaningful return value or copied lookup table."], "reason": "The standalone destination span writer clips endpoints against the same polygon state and performs live unsigned-byte lookup-table writes."}
// symbol    ?draw_remapped_polygon_span@@YAXHH@Z
// Hand-written assembly: explicit push edi/pop edi pair, one-operand imul, byte remap loop.
MEASURED inline void __cdecl draw_remapped_polygon_span(int left, int right) {
    __asm {
        push edi
        mov ebx, right
        cmp ebx, PolygonClipLeft
        jle span_done
        mov ecx, left
        cmp ecx, PolygonClipRight
        jge span_done
        sub ebx, ecx
        jle span_done
        mov ecx, right
        cmp ecx, PolygonClipRight
        jl right_clipped
        mov ecx, PolygonClipRight
        mov right, ecx
    right_clipped:
        mov edi, left
        cmp edi, PolygonClipLeft
        jge left_clipped
        mov edi, PolygonClipLeft
        mov left, edi
    left_clipped:
        sub ecx, edi
        jle span_done
        mov eax, PolygonY.coordinate
        imul PolygonPitch
        add eax, left
        add eax, PolygonPixels
        mov edi, eax
        mov ebx, PolygonRemap
    span_remap:
        xor edx, edx
        mov dl, [edi]
        add edx, ebx
        mov al, [edx]
        mov [edi], al
        inc edi
        dec ecx
        jne span_remap
    span_done:
        pop edi
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
        // The image tests the clip rectangle's address before reading it.
        RECT *clip = &buffer->rect1_;
        if (clip != 0) {
            int maximum_y = -32765;
            int minimum_vertex;
            PolygonClipRight = clip->right - 1;
            PolygonClipLeft = clip->left;
            PolygonClipBottom.coordinate = clip->bottom - 1;
            PolygonClipTop.coordinate = clip->top;
            int minimum_y = 32767;
            {
                for (int vertex = 0; vertex < count; ++vertex) {
                    if (vertices[vertex].y < minimum_y) {
                        minimum_y = vertices[vertex].y;
                        minimum_vertex = vertex;
                    }
                    if (vertices[vertex].y > maximum_y) {
                        // volatile: stand-in for the image's reload of vertices[vertex].y at 0x00626EC1; no plain spelling found
                        maximum_y = *(volatile int *)&vertices[vertex].y;
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
                                // Edge advance is open-coded: the image tests the
                                // raw setup result, without advance's normalization.
                                if (--backward.remaining_rows == 0) {
                                    if (!setup_flat_polygon_edge(&backward, backward.next_vertex)) {
                                        break;
                                    }
                                } else {
                                    backward.x += backward.x_step;
                                    backward.error += backward.remainder;
                                    if (backward.error > 0) {
                                        backward.x += backward.x_direction;
                                        backward.error -= backward.height;
                                    }
                                }
                                if (--forward.remaining_rows == 0) {
                                    if (!setup_flat_polygon_edge(&forward, forward.next_vertex)) {
                                        break;
                                    }
                                } else {
                                    forward.x += forward.x_step;
                                    forward.error += forward.remainder;
                                    if (forward.error > 0) {
                                        forward.x += forward.x_direction;
                                        forward.error -= forward.height;
                                    }
                                }
                                ++PolygonY.coordinate;
                            } while (PolygonY.coordinate < PolygonClipBottom.coordinate);
                        }
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


// RECOVERY_OBJECT: v1 0x00670C00 _PolygonLightScale
static const float PolygonLightScale = 256.0f;

// This setup uses the existing fifteen-word EdgeScan representation, with
// its four UV words untouched. Matching layout is not proof that the
// original polygon and texture code used this historical C++ type name.
// 0x00626900 reads direction/remaining/next at +0/+4/+8; x and its DDA
// occupy +1C/+20/+24/+28/+2C/+30, and light/step occupy +34/+38. The
// caller's two stack records are 0x3C bytes apart. Texture's declarations
// name those same fields without fabricating values for the unused UVs.
// ORIGINAL: 0x00626900 sub_626900 0x00626900-0x006269F3 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["626900..6269F3 uses stack edge/index arguments, wraps indices0..3 and initializes x at+1C, light at+34 and fixed_div628AD0 light step at+38. The shared type spelling is a representation choice, not proof of an original type name."], "reason": "This cdecl initializer consumes the shared four-vertex polygon and light arrays using the existing fifteen-word EdgeScan representation; it leaves the four unused interpolant words untouched."}
// symbol    ?setup_gouraud_polygon_edge@@YAHPAUEdgeScan@@H@Z
int __cdecl setup_gouraud_polygon_edge(EdgeScan *edge, int vertex) {
    int next;
    for (;;) {
        if (vertex == PolygonBottomVertex) {
            return 0;
        }
        next = vertex + edge->f0;
        if (next > 3) {
            next = 0;
        } else if (next < 0) {
            next = 3;
        }
        edge->counter = PolygonVertices[next].y - PolygonVertices[vertex].y;
        if (edge->counter != 0) {
            break;
        }
        vertex = next;
    }
    if (edge->counter <= 0) {
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
    int minimum_y;
    int maximum_y;
    int minimum_x;
    int maximum_x;
    int minimum_vertex;
    EdgeScan backward;
    EdgeScan forward;
    unsigned short control;
    int width;
    int delta;
    int done;
    if (vertices == 0 || lights == 0) {
        return 16;
    }
    PolygonPixels = static_cast<unsigned char *>(buffer->get_data());
    if (PolygonPixels != 0) {
        // Hand-written assembly: the rasterizer resets the FPU with FINIT.
        __asm finit
        PolygonVertices = vertices;
        PolygonLights = lights;
        PolygonPitch = static_cast<int>(buffer->stride_);
        RECT *clip = &buffer->rect1_;
        if (clip != 0) {
            PolygonClipRight = clip->right;
            PolygonClipLeft = clip->left;
            PolygonClipBottom.coordinate = clip->bottom;
            PolygonClipTop.coordinate = clip->top;
            minimum_y = 0x7fff;
            maximum_y = -0x7ffd;
            maximum_x = -0x7ffd;
            minimum_x = 0x7fff;
            for (int vertex = 0; vertex < 4; ++vertex) {
                if (vertices[vertex].y < minimum_y) {
                    minimum_y = vertices[vertex].y;
                    minimum_vertex = vertex;
                }
                if (vertices[vertex].y > maximum_y) {
                    maximum_y = vertices[vertex].y;
                    PolygonBottomVertex = vertex;
                }
                if (vertices[vertex].x < minimum_x) {
                    minimum_x = vertices[vertex].x;
                }
                if (vertices[vertex].x > maximum_x) {
                    maximum_x = vertices[vertex].x;
                }
            }
            if (minimum_y < maximum_y) {
                const int pitch = PolygonPitch;
                const int base = reinterpret_cast<int>(PolygonPixels);
                PolygonY.coordinate = pitch * minimum_y + base;
                PolygonClipBottom.coordinate = PolygonClipBottom.coordinate * pitch + base;
                PolygonClipTop.coordinate = PolygonClipTop.coordinate * pitch + base;
                backward.f0 = -1;
                if (setup_gouraud_polygon_edge(&backward, minimum_vertex)) {
                    forward.f0 = 1;
                    if (setup_gouraud_polygon_edge(&forward, minimum_vertex)) {
                        // Hand-written assembly: truncating rounding, 24-bit
                        // precision control word.
                        __asm {
                            fstcw control
                            mov ax, control
                            and ax, 0xfcff
                            or ax, 0xc00
                            mov control, ax
                            fldcw control
                        }
                        width = (forward.accumC - backward.accumC) << 16;
                        if (width > 0) {
                            delta = forward.accumD - backward.accumD;
                            // Hand-written assembly: the light slope stays on
                            // the x87 stack until the next row stores it.
                            __asm {
                                fild delta
                                fild width
                                fdivp st(1), st
                            }
                        }
                        if (PolygonY.coordinate < PolygonClipBottom.coordinate) {
                            done = 0;
                            for (;;) {
                                PolygonSpanRight = forward.accumC;
                                PolygonSpanLeft = backward.accumC;
                                PolygonLight = backward.accumD;
                                if (--backward.counter == 0) {
                                    if (!setup_gouraud_polygon_edge(&backward, backward.id)) {
                                        done = 1;
                                        goto edges_done;
                                    }
                                } else {
                                    backward.accumC += backward.deltaC;
                                    backward.accumD += backward.deltaD;
                                    backward.fracAccum += backward.fracDelta;
                                    if (backward.fracAccum > 0) {
                                        backward.accumC += backward.extraDeltaC;
                                        backward.fracAccum -= backward.fracSub;
                                    }
                                }
                                if (--forward.counter == 0) {
                                    if (!setup_gouraud_polygon_edge(&forward, forward.id)) {
                                        done = 1;
                                    }
                                } else {
                                    forward.accumC += forward.deltaC;
                                    forward.accumD += forward.deltaD;
                                    forward.fracAccum += forward.fracDelta;
                                    if (forward.fracAccum > 0) {
                                        forward.accumC += forward.extraDeltaC;
                                        forward.fracAccum -= forward.fracSub;
                                    }
                                }
                            edges_done:
                                if (width > 0) {
                                    // Hand-written assembly: scale the light
                                    // slope and store it as a 16-bit step.
                                    __asm {
                                        fmul PolygonLightScale
                                        fistp PolygonLightStep
                                    }
                                }
                                width = (forward.accumC - backward.accumC) << 16;
                                if (width > 0) {
                                    delta = forward.accumD - backward.accumD;
                                    __asm {
                                        fild delta
                                        fild width
                                        fdivp st(1), st
                                    }
                                }
                                if (PolygonY.coordinate >= PolygonClipBottom.coordinate) {
                                    break;
                                }
                                if (PolygonY.coordinate >= PolygonClipTop.coordinate) {
                                    // Hand-written assembly: clipped span with
                                    // a shrd light skip and an AH pixel store.
                                    __asm {
                                    mov ebx, PolygonSpanRight
                                    mov ecx, PolygonSpanLeft
                                    cmp ebx, PolygonClipLeft
                                    jle span_done
                                    cmp ecx, PolygonClipRight
                                    jge span_done
                                    sub ebx, ecx
                                    jle span_done
                                    shl ebx, 0x10
                                    mov ebx, PolygonSpanRight
                                    cmp ebx, PolygonClipRight
                                    jl right_clipped
                                    mov ebx, PolygonClipRight
                                right_clipped:
                                    mov edi, PolygonSpanLeft
                                    cmp edi, PolygonClipLeft
                                    jge left_clipped
                                    sub edi, PolygonClipLeft
                                    neg edi
                                    xor eax, eax
                                    mov ax, PolygonLightStep
                                    imul edi
                                    shrd eax, edx, 8
                                    add eax, PolygonLight
                                    mov PolygonLight, eax
                                    mov edi, PolygonClipLeft
                                    mov PolygonSpanLeft, edi
                                left_clipped:
                                    sub ebx, edi
                                    mov ecx, ebx
                                    mov edi, dword ptr [PolygonY]
                                    add edi, PolygonSpanLeft
                                    mov eax, PolygonLight
                                    shr eax, 8
                                    mov bx, PolygonLightStep
                                pixel:
                                    mov byte ptr [edi], ah
                                    add ax, bx
                                    inc edi
                                    dec ecx
                                    jns pixel
                                span_done:
                                    }
                                }
                                if (done) {
                                    break;
                                }
                                PolygonY.coordinate += PolygonPitch;
                            }
                        }
                    }
                }
            }
        }
    }
    buffer->free_data(1);
    __asm finit
    return 0;
}
