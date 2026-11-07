/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2021 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"

#include "general.h"  // range is used by veh.h inline members.
#include "veh.h"
#include "buffer.h"
#include "caviar.h"
#include "game.h"
#include "map.h"
#include "mapwin.h"
#include "console.h"
#include "designwin.h"
#include "tutwin.h"  // TutWinDesWindow is the crawler-preview consumer.
#include "interface_palette.h"
#include "vector.h"
#include "technology.h"

// Vehicle renderer lifetime is later than model, terrain and sprite assets.
// ORIGINAL: 0x004BF6E0 _$E21 0x004BF6E0-0x004BF6F6 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x006825FC selects init 0x004BF6E0-0x004BF6F6, registering fini 0x004BF700-0x004BF70A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x008CC828; scalar Caviar constructor/destructor 0x00616DA0/0x00617020.", "Physical startup-root, registered-cleanup and global relocations identify this source-owned object in src/vehicle_render.cpp. The adjacent symbol fact names the compiler-emitted operation. Out-of-line class methods supply construction and teardown; no handwritten lifecycle callback or fixed-address object is substituted.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned g_VEHDRAW_CAVIAR; no callback body is authored."}
// symbol    _$E16
// ORIGINAL: 0x004BF700 _$E19 0x004BF700-0x004BF70A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x006825FC selects init 0x004BF6E0-0x004BF6F6, registering fini 0x004BF700-0x004BF70A; complete independent instruction decode establishes both ends.", "Both pinned operations identify storage 0x008CC828; scalar Caviar constructor/destructor 0x00616DA0/0x00617020.", "Physical startup-root, registered-cleanup and global relocations identify this source-owned object in src/vehicle_render.cpp. The adjacent symbol fact names the compiler-emitted operation. Out-of-line class methods supply construction and teardown; no handwritten lifecycle callback or fixed-address object is substituted.", "Current helper names are compiler-output interpretations, not original debug symbols. These individual bindings do not establish full initialization order, allocation extents, linked retained selection or runtime acceptance."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned g_VEHDRAW_CAVIAR; no callback body is authored."}
// symbol    _$E14

// Original CRT slots 0x00682104..0x00682288 construct all 98 model-data
// globals before the renderer at slot 0x006825FC. Preserve that relative
// lifetime order here; this does not establish whole-game startup ordering.
// turn_start addresses this renderer at 0x008CC828 and its light/shadow
// vectors at +0x60/+0x6C. It is the existing C++ static owner, not an alias.
// RECOVERY_OBJECT: v1 0x008CC828 ?g_VEHDRAW_CAVIAR@@3VCaviar@@A
Caviar g_VEHDRAW_CAVIAR;

// ORIGINAL: 0x004BF710 sub_4bf710 0x004BF710-0x004BF726 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682600 selects initializer 0x004BF710..0x004BF726 for Buffer at 0x008CC298 and registers finalizer 0x004BF730..0x004BF73A. Cleanup unconditionally tail-calls Buffer::~Buffer; resource-state checks belong to Buffer::close, not an added global teardown guard.", "Physical constructor, atexit-registration and destructor relocation graphs identify g_VEHDRAW_BUFFER independently of private label numbering. The ordinary g_BUFFER at 0x009B6080 is a separate lifetime; same-shaped code does not establish receiver identity.", "Original identities, complete spans and historical claims remain unchanged. Vehicle-rendering ownership is supported by its consumers and adjacent renderer state; complete historical TU boundaries, linked placement and game acceptance remain unproved. This binding grants no claim."], "reason": "Bind the original vehicle-rendering Buffer lifetime to its actual typed global and compiler-generated callbacks."}
// symbol    _$E21
// ORIGINAL: 0x004BF730 sub_4bf730 0x004BF730-0x004BF73A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot 0x00682600 selects initializer 0x004BF710..0x004BF726 for Buffer at 0x008CC298 and registers finalizer 0x004BF730..0x004BF73A. Cleanup unconditionally tail-calls Buffer::~Buffer; resource-state checks belong to Buffer::close, not an added global teardown guard.", "Physical constructor, atexit-registration and destructor relocation graphs identify g_VEHDRAW_BUFFER independently of private label numbering. The ordinary g_BUFFER at 0x009B6080 is a separate lifetime; same-shaped code does not establish receiver identity.", "Original identities, complete spans and historical claims remain unchanged. Vehicle-rendering ownership is supported by its consumers and adjacent renderer state; complete historical TU boundaries, linked placement and game acceptance remain unproved. This binding grants no claim."], "reason": "Bind the original vehicle-rendering Buffer lifetime to its actual typed global and compiler-generated callbacks."}
// symbol    _$E19

// Vehicle-rendering scratch Buffer: CRT slot 0x00682600 selects startup
// 0x004BF710 and registered cleanup 0x004BF730. Its 0x588-byte extent ends
// at 0x008CC820, the independently mapped VehDrawTurnVehicle state.
// Interface PCX, array and popup buffers are owned by interface_assets.cpp;
// their separate lifetimes do not establish this unit's complete CRT order.
// RECOVERY_OBJECT: v1 0x008CC298 ?g_VEHDRAW_BUFFER@@3VBuffer@@A
Buffer g_VEHDRAW_BUFFER;  // 0x008CC298




// Four normal tables, each indexed by the four terrain-height levels in
// vehdraw_compute_facet_normals. One object: the initializer walks all four
// rows from one pointer (-0x708C, +0x708C, +0xE118), and readers fold row
// offsets into displacements (row 1 at [index - 400] is 0x008D3ADC).
// RECOVERY_OBJECT: v2 0x008CDD10 0x008E9F40 ?VehDrawFacetNormals@@3PAY0JGB@UVOX_Vect@@A 0x004C1B10
VOX_Vect VehDrawFacetNormals[4][2401];

// compute_camera publishes the first rotation here; vehicle rendering reads
// its first component when converting facing angles into camera coordinates.
// RECOVERY_OBJECT: v2 0x008E9F40 0x008E9F4C ?VehDrawCameraAngles@@3UVOX_Vect@@A 0x004BF740
VOX_Vect VehDrawCameraAngles;

// load_voxels reads VEH_LIGHT_DIR_MAP, converts its three integer settings
// to floats and normalizes this vector. These are the image's defaults.
// RECOVERY_OBJECT: v2 0x00687AD8 0x00687AE4 ?VehLightDirMap@@3UVOX_Vect@@A 0x004BF8D0
VOX_Vect VehLightDirMap = {{-0.57735f, 0.57735f, 0.57735f}};
// load_voxels derives these offsets from the first part's two step vectors:
// AVGMTGUN, adroper, AVHT-GunReposition, and AVHT-LightsReposition.
// RECOVERY_OBJECT: v2 0x007AC1B0 0x007AC1BC ?VehGunMountOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehGunMountOffset;    // 0x007AC1B0
// RECOVERY_OBJECT: v2 0x0079A618 0x0079A624 ?VehDropOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehDropOffset;        // 0x0079A618
// RECOVERY_OBJECT: v2 0x00798C30 0x00798C3C ?VehHoverGunOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehHoverGunOffset;    // 0x00798C30
// RECOVERY_OBJECT: v2 0x00789B48 0x00789B54 ?VehHoverLightsOffset@@3UVOX_Vect@@A 0x00451030
VOX_Vect VehHoverLightsOffset; // 0x00789B48

// Animation frame read by model assembly. Original 8E9F50 starts at zero;
// scoot_start sets it to one, and visible crawler assembly advances it.
// RECOVERY_OBJECT: v2 0x008E9F50 0x008E9F54 ?VehDrawAnimationStep@@3HA 0x004C2470
int VehDrawAnimationStep;


// Turn setup publishes these for the subsequent animation frames.
// RECOVERY_OBJECT: v2 0x008CC820 0x008CC824 ?VehDrawTurnVehicle@@3HA 0x004BF8D0
int VehDrawTurnVehicle;

// RECOVERY_OBJECT: v2 0x008CC260 0x008CC264 ?VehDrawTurnAngle@@3MA 0x004BF8D0
float VehDrawTurnAngle;

// RECOVERY_OBJECT: v2 0x008CC258 0x008CC25C ?VehDrawTurnIncrement@@3MA 0x004BF8D0
float VehDrawTurnIncrement;

// These are independent animation globals, not fields beyond the renderer's
// complete-object boundary. Scoot setup publishes two normal increments and
// their respective origins; each frame interpolates only its selected half.
// RECOVERY_OBJECT: v2 0x008CC25C 0x008CC260 ?VehDrawScootFacing@@3HA 0x004BFE00
int VehDrawScootFacing;
// RECOVERY_OBJECT: v2 0x008CDCF8 0x008CDCFC ?VehDrawScootSteps@@3HA 0x004BFE00
int VehDrawScootSteps;
// RECOVERY_OBJECT: v2 0x008CC268 0x008CC274 ?VehDrawScootFirstIncrement@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootFirstIncrement;
// RECOVERY_OBJECT: v2 0x008CC278 0x008CC284 ?VehDrawScootSecondIncrement@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootSecondIncrement;
// RECOVERY_OBJECT: v2 0x008CC288 0x008CC294 ?VehDrawScootNormalMid@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootNormalMid;
// RECOVERY_OBJECT: v2 0x008CDD00 0x008CDD0C ?VehDrawScootNormalStart@@3UVOX_Vect@@A 0x004BFE00
VOX_Vect VehDrawScootNormalStart;

// Temporary draw coordinates belong to the vehicle rendering state; -1 means
// no override. veh_draw uses them only when the selected vehicle also agrees.
// RECOVERY_OBJECT: v2 0x0068E564 0x0068E568 ?VehDrawPositionX@@3HA 0x004C12A0
int VehDrawPositionX = -1;

// RECOVERY_OBJECT: v2 0x0068E568 0x0068E56C ?VehDrawPositionY@@3HA 0x004C12A0
int VehDrawPositionY = -1;

// RECOVERY_OBJECT: v2 0x0068E56C 0x0068E570 ?VehDrawPositionVehicle@@3HA 0x004C12A0
int VehDrawPositionVehicle = -1;

static float vehdraw_scale(int zoom) {
    if (MapWinCaptionScale > 0) {
        return MapWinCaptionScale * 0.07f / MapWinCaptionDivisor;
    }
    float scale = (zoom + 16) * 0.07f;
    scale *= 0.0625f;
    return scale;
}


static int vehdraw_centre_offset(int numerator, int zoom) {
    if (numerator > 0) {
        return (numerator * 50) / MapWinCaptionDivisor;
    }
    return ((zoom * 5 + 80) * 10) / 16;
}


static bool vehdraw_uses_terrain(int veh_id, int x, int y) {
    const Map *tiles = map_tiles();
    if ((tiles[y * MapLongitude + (x >> 1)].climate & ~0x1F) < 0x60) {
        return false;
    }
    return VehPrototypes[Vehs[veh_id].proto_id].chassis_id != 0
        || Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].mode >= 3;
}


static int vehdraw_facet_index(const int *h1, const int *h2,
                               const int *h3, const int *h4, int cell) {
    return ((h1[cell] * 7 + h2[cell]) * 7 + h3[cell]) * 7 + h4[cell] - 400;
}


static void vehdraw_average_facets(int index, VOX_Vect *normal) {
    // Copy every input before the first call, as the original does.
    const VOX_Vect n0 = VehDrawFacetNormals[0][index];
    const VOX_Vect n1 = VehDrawFacetNormals[1][index];
    const VOX_Vect n2 = VehDrawFacetNormals[2][index];
    const VOX_Vect n3 = VehDrawFacetNormals[3][index];
    vector_add(n0.values, n1.values, normal->values);
    vector_add(normal->values, n2.values, normal->values);
    vector_add(normal->values, n3.values, normal->values);
    vox_normalise_vector(normal->values, normal->values);
}


static void vehdraw_finish_frame(Buffer *buffer, int veh_id, int x, int y, int flags) {
    // Rendering can change the selected prototype; fetch it after render.
    if (VehPrototypes[Vehs[veh_id].proto_id].armor_id == 5) {
        g_VEHDRAW_CAVIAR.reset_objects();
        vehdraw_construct_armor(Vehs[veh_id].faction_id,
                               VehPrototypes[Vehs[veh_id].proto_id].chassis_id,
                               VehPrototypes[Vehs[veh_id].proto_id].weapon_id,
                               5, 1, &g_VEHDRAW_CAVIAR);
        g_VEHDRAW_CAVIAR.render_multi_table_dest(buffer, x, y, 24,
            ArmorTransparentColorTables, flags);
    } else {
        g_VEHDRAW_CAVIAR.clear_buffers(0);
    }
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BF740 ?compute_camera@@YAXPAUVOX_Vect@@PAUVOX_Matrix@@@Z 0x004BF740-0x004BF8C1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original control flow and original seed extent; observed VC6 external symbol. Tests exercise actual bodies, call/publication order, unordered/zero camera branches and complete facet tables at both established baselines.", "Camera publishes its first Euler triple before the second rotation; original CRT atan calls round their quotient to double. Facet initialization snapshots ConsoleGlobal inherited MapWin field at1DD6C, established by receiver-relative MapWin initialization; four independent 2401-element tables replace cross-object pointer arithmetic."], "reason": "Vehicle rendering setup with real VOX types, source-owned tables and actual Console terrain-height state."}
// The image calls the CRT atan rather than expanding fpatan.
#pragma function(atan)
void __cdecl compute_camera(VOX_Vect *normal, VOX_Matrix *matrix) {
    VOX_Vect angles = {{0.0f, 0.0f, -1.0466667f}};
    VOX_Matrix first_rotation;
    VOX_Matrix second_rotation;
    VOX_Vect rotated;
    caviar_apply_rotation(angles.values, &first_rotation);
    vox_matrix_vector_mul(first_rotation.values, normal->values, rotated.values);
    // The image copies the whole result here and rebuilds it below.
    VOX_Vect projected = rotated;

    // VC6's x87 equality test also takes the zero arm for unordered values.
    if (rotated.values[0] == 0.0f) {
        angles.values[0] = 0.0f;
    } else if (rotated.values[1] == 0.0f) {
        angles.values[0] = 1.57f;
    } else {
        angles.values[0] = (float)atan((double)rotated.values[0] / rotated.values[1]);
    }
    angles.values[1] = 0.0f;
    angles.values[2] = 0.0f;
    VehDrawCameraAngles = angles;
    caviar_apply_rotation(angles.values, &first_rotation);

    projected.values[0] = rotated.values[0];
    projected.values[1] = rotated.values[1];
    projected.values[2] = 0.0f;
    float length = vec3_length(projected.values);
    if (length == 0.0f) {
        angles.values[2] = 0.0f;
    } else if (rotated.values[2] == 0.0f) {
        angles.values[2] = 1.57f;
    } else {
        angles.values[2] = (float)atan((double)length / rotated.values[2]);
    }
    angles.values[0] = 0.0f;
    angles.values[1] = 0.0f;
    caviar_apply_rotation(angles.values, &second_rotation);
    vox_matrix_matrix_mul(first_rotation.values, second_rotation.values, matrix->values);
    matrix->values[2][0] = -matrix->values[2][0];
    matrix->values[2][1] = -matrix->values[2][1];
    matrix->values[2][2] = -matrix->values[2][2];
}

#pragma intrinsic(atan)

// ORIGINAL: 0x004BF8D0 ?vehdraw_turn_start@@YAHHHHH@Z 0x004BF8D0-0x004BFC67
// RECOVERY_BINDING: v1 {"evidence": ["Original4BF8D0..4BFC67 returns the integer animation step count, snapshots light before selection, publishes selection before facing access, loads the map pointer slot, and uses unsigned Weapon.mode at+0A.", "All four facet vectors are copied before calls; camera/light/shadow publication follows original call order. Original stores only negative arc to float before comparison; preserving positive precision determines half-turn direction. Zero angle preserves zero steps and NaN increment. Observed VC6 emitted symbol and layouts."], "reason": "Recover vehicle turn setup using actual map, vehicle, renderer and vector ownership instead of fixed executable storage."}
int __cdecl vehdraw_turn_start(int veh_id, int direction, int speed, int flags) {
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    const VOX_Vect light = VehLightDirMap;
    if (flags & 1) {
        veh_id = pick_top_veh(veh_id);
    }
    VehDrawTurnVehicle = veh_id;
    const int x = VehLiftX;
    const int y = VehLiftY;
    int old_facing = Vehs[veh_id].facing;
    if (old_facing == -1) {
        old_facing = 0;
    }
    Vehs[veh_id].facing = (int8_t)direction;

    // One scratch vector serves as facet 0, the shadow and the camera origin.
    VOX_Vect vector;
    VOX_Vect normal;
    if ((map_tiles()[y * MapLongitude + (x >> 1)].climate & ~0x1F) >= 0x60
        && (VehPrototypes[Vehs[veh_id].proto_id].chassis_id != 0
            || Weapon[VehPrototypes[Vehs[veh_id].proto_id].weapon_id].mode >= 3)) {
        const int h1 = ConsoleGlobal.get_relative_alt(x, y, 1);
        const int h2 = ConsoleGlobal.get_relative_alt(x, y, 2);
        const int h3 = ConsoleGlobal.get_relative_alt(x, y, 3);
        const int h4 = ConsoleGlobal.get_relative_alt(x, y, 4);
        const int index = h4 + ((h1 * 7 + h2) * 7 + h3) * 7;
        vector = VehDrawFacetNormals[0][index - 400];
        const VOX_Vect n1 = VehDrawFacetNormals[1][index - 400];
        const VOX_Vect n2 = VehDrawFacetNormals[2][index - 400];
        const VOX_Vect n3 = VehDrawFacetNormals[3][index - 400];
        vector_add(vector.values, n1.values, normal.values);
        vector_add(normal.values, n2.values, normal.values);
        vector_add(normal.values, n3.values, normal.values);
        vox_normalise_vector(normal.values, normal.values);
    } else {
        normal.values[0] = 0.0f;
        normal.values[1] = 0.0f;
        normal.values[2] = 1.0f;
    }

    VOX_Matrix camera;
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector() = light;
    VOX_Matrix inverse;
    vox_matrix_inverse(camera.values, inverse.values);
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, vector.values);
    g_VEHDRAW_CAVIAR.shadow_vector() = vector;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    vector.values[0] = camera.values[2][0] * -10000.0f;
    vector.values[1] = camera.values[2][1] * -10000.0f;
    vector.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_camera_direct(&vector, &camera);

    VehDrawTurnAngle = ((old_facing - 3) & 7) * 0.785375f;
    const float target = ((direction - 3) & 7) * 0.785375f;
    float negative_distance;
    // VC6 rounds the negative arc to memory but keeps the positive arc in
    // an x87 register (extended precision) through the comparison.
    float positive_distance;
    if (VehDrawTurnAngle > target) {
        negative_distance = VehDrawTurnAngle - target;
        positive_distance = target + 6.283f - VehDrawTurnAngle;
    } else {
        positive_distance = target - VehDrawTurnAngle;
        negative_distance = VehDrawTurnAngle + 6.283f - target;
    }
    if (negative_distance < positive_distance) {
        const int steps = (int)(speed * negative_distance * 0.15915963f);
        VehDrawTurnIncrement = -(negative_distance / steps);
        return steps;
    }
    const int steps = (int)(speed * positive_distance * 0.15915963f);
    VehDrawTurnIncrement = positive_distance / steps;
    return steps;
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004BFC70 ?vehdraw_turn@@YAXPAUBuffer@@HHHH@Z 0x004BFC70-0x004BFDFA BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original selects scale, rotates scene from frame/turn increment, computes caption-centred coordinates, resets and assembles the actual vehicle, renders, then reloads its prototype for transparent-armor overlay or buffer clearing.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover a vehicle turn frame through the real renderer and shared turn state."}
// symbol    ?vehdraw_turn@@YAXPAVBuffer@@HHHH@Z
void __cdecl vehdraw_turn(Buffer *buffer, int x, int y, int frame, int zoom) {
    const int veh_id = VehDrawTurnVehicle;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        frame * VehDrawTurnIncrement + VehDrawTurnAngle - VehDrawCameraAngles.values[0],
        0.0f, 0.0f);
    const int numerator = MapWinCaptionScale;
    const int draw_x = x + vehdraw_centre_offset(numerator, zoom);
    const int draw_y = y + vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.reset_objects();
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    g_VEHDRAW_CAVIAR.render(buffer, draw_x, draw_y, 1);
    vehdraw_finish_frame(buffer, veh_id, draw_x, draw_y, 0);
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x004BFE00 0x004C0F24 0x004C0F44 0x004C02BA
// ORIGINAL: 0x004BFE00 ?vehdraw_scoot_start@@YAXHHHH@Z 0x004BFE00-0x004C0F24
// RECOVERY_BINDING: v1 {"evidence": ["Original samples four nine-element altitude arrays, including the zero-delta centre; it snapshots original facet vectors before additions. Even directions combine destination facet3 and centre facet1; odd directions use eight ordered facets. Both interpolation increments divide by their respective half-frame counts.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover terrain-normal interpolation setup using nine actual terrain samples and independent animation globals."}
// symbol    ?vehdraw_scoot_start@@YAXHHHH@Z
void __cdecl vehdraw_scoot_start(int veh_id, int direction, int steps, int flags) {
    if (flags & 1) veh_id = pick_top_veh(veh_id);
    VehDrawScootSteps = steps;
    VehDrawTurnVehicle = veh_id;
    VehDrawScootFacing = (direction - 3) & 7;

    // The ninth sample is the centre. The four original stack runs each
    // occupy nine integers, including the values used for the starting tile.
    int h1[9], h2[9], h3[9], h4[9];
    int x, y;
    for (int cell = 0; cell < 9; ++cell) {
        x = wrap_map_x(VehMoveDeltaX[cell] + VehLiftX);
        y = VehMoveDeltaY[cell] + VehLiftY;
        h1[cell] = ConsoleGlobal.get_relative_alt(x, y, 1);
        h2[cell] = ConsoleGlobal.get_relative_alt(x, y, 2);
        h3[cell] = ConsoleGlobal.get_relative_alt(x, y, 3);
        h4[cell] = ConsoleGlobal.get_relative_alt(x, y, 4);
    }
    VOX_Vect start, end, midpoint;
    if (vehdraw_uses_terrain(veh_id, x, y)) {
        vehdraw_average_facets(vehdraw_facet_index(h1, h2, h3, h4, 8), &start);
    } else {
        start.values[0] = start.values[1] = 0.0f;
        start.values[2] = 1.0f;
        midpoint = start;
    }
    x = VehMoveDeltaX[direction] + VehLiftX;
    VehDrawAnimationStep = 1;
    x = wrap_map_x(x);
    y = VehMoveDeltaY[direction] + VehLiftY;
    if (vehdraw_uses_terrain(veh_id, x, y)) {
        vehdraw_average_facets(vehdraw_facet_index(h1, h2, h3, h4, direction), &end);
    } else {
        end.values[0] = end.values[1] = 0.0f;
        end.values[2] = 1.0f;
    }
    // Re-read terrain/prototype after the normalizer calls. The midpoint is
    // the shared edge (even directions) or the eight meeting facets (odd).
    if (!vehdraw_uses_terrain(veh_id, x, y)) {
        midpoint.values[0] = midpoint.values[1] = 0.0f;
        midpoint.values[2] = 1.0f;
    } else {
        const int centre = vehdraw_facet_index(h1, h2, h3, h4, 8);
        VOX_Vect n[8];
        int count = 0;
        switch (direction) {
        case 0: case 2: case 4: case 6:
            n[0] = VehDrawFacetNormals[3][vehdraw_facet_index(h1, h2, h3, h4, direction)];
            n[1] = VehDrawFacetNormals[1][centre];
            count = 2;
            break;
        case 1: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 0);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 1);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 2);
            n[0] = VehDrawFacetNormals[1][centre]; n[1] = VehDrawFacetNormals[2][centre];
            n[2] = VehDrawFacetNormals[2][a]; n[3] = VehDrawFacetNormals[3][a];
            n[4] = VehDrawFacetNormals[3][b]; n[5] = VehDrawFacetNormals[0][b];
            n[6] = VehDrawFacetNormals[0][c]; n[7] = VehDrawFacetNormals[1][c];
            count = 8;
            break;
        }
        case 3: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 2);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 3);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 4);
            n[0] = VehDrawFacetNormals[2][centre]; n[1] = VehDrawFacetNormals[3][centre];
            n[2] = VehDrawFacetNormals[3][a]; n[3] = VehDrawFacetNormals[0][a];
            n[4] = VehDrawFacetNormals[0][b]; n[5] = VehDrawFacetNormals[1][b];
            n[6] = VehDrawFacetNormals[1][c]; n[7] = VehDrawFacetNormals[2][c];
            count = 8;
            break;
        }
        case 5: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 4);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 5);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 6);
            n[0] = VehDrawFacetNormals[3][centre]; n[1] = VehDrawFacetNormals[0][centre];
            n[2] = VehDrawFacetNormals[0][a]; n[3] = VehDrawFacetNormals[1][a];
            n[4] = VehDrawFacetNormals[1][b]; n[5] = VehDrawFacetNormals[2][b];
            n[6] = VehDrawFacetNormals[2][c]; n[7] = VehDrawFacetNormals[3][c];
            count = 8;
            break;
        }
        case 7: {
            const int a = vehdraw_facet_index(h1, h2, h3, h4, 6);
            const int b = vehdraw_facet_index(h1, h2, h3, h4, 7);
            const int c = vehdraw_facet_index(h1, h2, h3, h4, 0);
            n[0] = VehDrawFacetNormals[0][centre]; n[1] = VehDrawFacetNormals[1][centre];
            n[2] = VehDrawFacetNormals[1][a]; n[3] = VehDrawFacetNormals[2][a];
            n[4] = VehDrawFacetNormals[2][b]; n[5] = VehDrawFacetNormals[3][b];
            n[6] = VehDrawFacetNormals[3][c]; n[7] = VehDrawFacetNormals[0][c];
            count = 8;
            break;
        }
        }
        if (count) {
            vector_add(n[0].values, n[1].values, midpoint.values);
            for (int i = 2; i < count; ++i) {
                vector_add(midpoint.values, n[i].values, midpoint.values);
            }
        }
    }
    vox_normalise_vector(midpoint.values, midpoint.values);
    vec3_subtract(midpoint.values, start.values, VehDrawScootFirstIncrement.values);
    vec3_subtract(end.values, midpoint.values, VehDrawScootSecondIncrement.values);
    const int half = steps >> 1;
    vec3_scale(VehDrawScootFirstIncrement.values, 1.0f / half,
                          VehDrawScootFirstIncrement.values);
    vec3_scale(VehDrawScootSecondIncrement.values, 1.0f / (steps - half),
                          VehDrawScootSecondIncrement.values);
    VehDrawScootNormalStart = start;
    VehDrawScootNormalMid = midpoint;
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C0F50 ?vehdraw_scoot@@YAXPAUBuffer@@HHHH@Z 0x004C0F50-0x004C1296 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original chooses first or second normal increment at a strict greater-than half-frame boundary, snapshots light, computes camera and shadow, sets scaling both before and after camera publication, assembles vehicle, increments animation, renders, and reloads armor state.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover an interpolated vehicle movement frame using real camera, normal, shadow and renderer ownership."}
// symbol    ?vehdraw_scoot@@YAXPAVBuffer@@HHHH@Z
void __cdecl vehdraw_scoot(Buffer *buffer, int x, int y, int frame, int zoom) {
    const int half = VehDrawScootSteps >> 1;
    const VOX_Vect light = VehLightDirMap;
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    // One scratch vector holds the base, then the shadow, then the position.
    VOX_Vect scratch, increment, normal;
    VOX_Matrix camera, inverse;
    if (frame > half) {
        frame -= half;
        scratch = VehDrawScootNormalMid;
        increment = VehDrawScootSecondIncrement;
    } else {
        scratch = VehDrawScootNormalStart;
        increment = VehDrawScootFirstIncrement;
    }
    vec3_scale(increment.values, (float)frame, increment.values);
    vector_add(increment.values, scratch.values, normal.values);
    vox_normalise_vector(normal.values, normal.values);
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector() = light;
    vox_matrix_inverse(camera.values, inverse.values);
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, scratch.values);
    g_VEHDRAW_CAVIAR.shadow_vector() = scratch;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    scratch.values[0] = camera.values[2][0] * -10000.0f;
    scratch.values[1] = camera.values[2][1] * -10000.0f;
    scratch.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_camera_direct(&scratch, &camera);
    // This second scale operation and fresh global reads occur in the image.
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        VehDrawScootFacing * 0.785375f - VehDrawCameraAngles.values[0], 0.0f, 0.0f);
    const int veh_id = VehDrawTurnVehicle;
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    ++VehDrawAnimationStep;
    const int numerator = MapWinCaptionScale;
    const int draw_x = x + vehdraw_centre_offset(numerator, zoom);
    const int draw_y = y + vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.render(buffer, draw_x, draw_y, 1);
    vehdraw_finish_frame(buffer, veh_id, draw_x, draw_y, 0);
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C12A0 ?vehdraw_draw@@YAXPAUBuffer@@HHHHHH@Z 0x004C12A0-0x004C1732
// RECOVERY_BINDING: v1 {"evidence": ["Original null-buffer return precedes work; optional stack selection and temporary coordinate override choose the vehicle/location. Terrain uses the actual map pointer slot and base7 VOX_Vect facet arrays. Vehicle assembly precedes scene rotation; render flags derive bit13 and transparent armor uses the corresponding overlay flags.", "Original mapped extent is preserved from the existing unclaimed seed and checked against the pinned executable; catalogue return/type names remain provenance until observed compiler-symbol facts are published."], "reason": "Recover ordinary vehicle voxel drawing through real terrain and model owners."}
// symbol    ?vehdraw_draw@@YAXPAVBuffer@@HHHHHH@Z
void __cdecl vehdraw_draw(Buffer *buffer, int veh_id, int x, int y, int flags,
                          int zoom, int terrain) {
    const VOX_Vect shadow_direction = {{-0.57735f, 0.57735f, 0.57735f}};
    if (!buffer) return;
    const VOX_Vect light = VehLightDirMap;
    if (flags & 1) veh_id = pick_top_veh(veh_id);
    int tile_x = VehDrawPositionX;
    int tile_y;
    if (tile_x == -1 || VehDrawPositionVehicle != veh_id) {
        tile_x = Vehs[veh_id].x;
        tile_y = Vehs[veh_id].y;
    } else {
        tile_y = VehDrawPositionY;
    }
    int facing = Vehs[veh_id].facing;
    if (facing == -1) facing = 0;
    // One scratch vector serves as facet 0, the shadow and the camera origin.
    VOX_Vect vector;
    VOX_Vect normal;
    if (terrain && vehdraw_uses_terrain(veh_id, tile_x, tile_y)) {
        const int h1 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 1);
        const int h2 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 2);
        const int h3 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 3);
        const int h4 = ConsoleGlobal.get_relative_alt(tile_x, tile_y, 4);
        const int index = h4 + ((h1 * 7 + h2) * 7 + h3) * 7;
        vector = VehDrawFacetNormals[0][index - 400];
        const VOX_Vect n1 = VehDrawFacetNormals[1][index - 400];
        const VOX_Vect n2 = VehDrawFacetNormals[2][index - 400];
        const VOX_Vect n3 = VehDrawFacetNormals[3][index - 400];
        vector_add(vector.values, n1.values, normal.values);
        vector_add(normal.values, n2.values, normal.values);
        vector_add(normal.values, n3.values, normal.values);
        vox_normalise_vector(normal.values, normal.values);
    } else {
        normal.values[0] = 0.0f;
        normal.values[1] = 0.0f;
        normal.values[2] = 1.0f;
    }
    VOX_Matrix camera, inverse;
    compute_camera(&normal, &camera);
    g_VEHDRAW_CAVIAR.light_vector() = light;
    vox_matrix_inverse(camera.values, inverse.values);
    vox_matrix_vector_mul(inverse.values, shadow_direction.values, vector.values);
    g_VEHDRAW_CAVIAR.shadow_vector() = vector;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);
    vector.values[0] = camera.values[2][0] * -10000.0f;
    vector.values[1] = camera.values[2][1] * -10000.0f;
    vector.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_scaling(vehdraw_scale(zoom));
    g_VEHDRAW_CAVIAR.set_camera_direct(&vector, &camera);
    vehdraw_construct_vehicle(veh_id, &g_VEHDRAW_CAVIAR);
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        ((facing - 3) & 7) * 0.785375f - VehDrawCameraAngles.values[0], 0.0f, 0.0f);
    const int numerator = MapWinCaptionScale;
    const int draw_x = x + vehdraw_centre_offset(numerator, zoom);
    const int draw_y = y + vehdraw_centre_offset(numerator, zoom);
    g_VEHDRAW_CAVIAR.render(buffer, draw_x, draw_y,
        ((static_cast<unsigned int>(flags) >> 12) & 2) | 1);
    vehdraw_finish_frame(buffer, veh_id, draw_x, draw_y,
        (static_cast<unsigned int>(flags) >> 12) & 2);
}


// ORIGINAL: 0x004C1740 vehdraw_unk1 0x004C1740-0x004C1B05 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["source:src/unrecovered/004c1740.cpp: preserved original identity/span and retained 18 call targets", "seed history preserved in the parsed source record and this binding: FILE mode, IMPLEMENTED state, kind=game, empty prototype, 0 callers, flags=frame;hidden;sp_ready;purged_ok, VERBATIM recipe; formal LEVER/TRIED/UNRECOVERABLE/DEFERRED lessons remain empty", "original call-target set:0x0046FE00,0x004BF740,0x004C1D20,0x004C3090,0x005593E0,0x005D7670,0x005DE8F0,0x006171A0,0x00617230,0x006182A0,0x00618370,0x006183B0,0x00618430,0x00618470,0x00627F50,0x006280E0,0x00628150,0x00628220", "source:src/veh.h: Veh 0x34 layout, Vehs, four 2401-record facet tables and renderer declarations", "source:src/veh.cpp: pick_top_veh, compute_camera, current vehicle construction/armor owners and render helpers", "source:src/console.h: ConsoleGlobal is a real MapWin subtype with get_relative_alt", "source:src/map.h: MapTiles/MapLongitude and Map::climate typed ownership", "source:src/buffer.cpp: g_VEHDRAW_BUFFER at 0x008CC298; Buffer+0x50 is typed locked_bits_", "source:src/caviar.cpp: g_VEHDRAW_CAVIAR and camera/render/reset APIs", "source:src/game.cpp and src/interface_palette.cpp: ShadowColorTable, ArmorTransparentColorTables and g_PALETTE1 owners", "docs/recovery/agent-structure-observations.csv: 0x004C1740 four-table vector-chain observation remains open"], "reason": "Recover the retained vehicle-renderer identity through the genuine Veh, MapWin/Console, Buffer, Palette and Caviar owners. Preserve the seed's terrain table asymmetry, render flag derivation, post-render prototype reread, transparent-armor path and unchecked failure/invalid-index behavior. Original decorated symbol and exact caller boundary remain unverified; this proposal is semantic source ownership, not native acceptance."}
// symbol    ?vehdraw_unk1@@YAXPAVBuffer@@HHHIHHH@Z
void __cdecl vehdraw_unk1(Buffer *buffer, int vehicle_id, int x, int y,
                         unsigned int flags, int scale_numerator,
                         int scale_denominator, int terrain) {
    if (buffer == 0) {
        return;
    }

    const unsigned int original_flags = flags;
    // Snapshot before stack selection, as the original does.
    const VOX_Vect camera_position = VehLightDirMap;
    if ((original_flags & 1) != 0) {
        vehicle_id = pick_top_veh(vehicle_id);
    }

    const Veh &vehicle = Vehs[vehicle_id];
    const int facing_x = vehicle.x;
    const int facing_y = vehicle.y;
    int frame = vehicle.facing;
    if (frame == -1) {
        frame = 0;
    }

    // One scratch vector serves as facet 0, the shadow and the camera origin.
    VOX_Vect vector;
    VOX_Vect light;
    if (terrain != 0
        && (map_tiles()[facing_y * MapLongitude + (facing_x >> 1)].climate
            & ~0x1F) >= 0x60) {
        const int r1 = ConsoleGlobal.get_relative_alt(facing_x, facing_y, 1);
        const int r2 = ConsoleGlobal.get_relative_alt(facing_x, facing_y, 2);
        const int r3 = ConsoleGlobal.get_relative_alt(facing_x, facing_y, 3);
        const int r4 = ConsoleGlobal.get_relative_alt(facing_x, facing_y, 4);
        const int index = r4 + ((r1 * 7 + r2) * 7 + r3) * 7;
        // All four tables take the -400 bias (table 1 at 0x8D3ADC, etc.).
        vector = VehDrawFacetNormals[0][index - 400];
        const VOX_Vect n1 = VehDrawFacetNormals[1][index - 400];
        const VOX_Vect n2 = VehDrawFacetNormals[2][index - 400];
        const VOX_Vect n3 = VehDrawFacetNormals[3][index - 400];
        vector_add(vector.values, n1.values, light.values);
        vector_add(light.values, n2.values, light.values);
        vector_add(light.values, n3.values, light.values);
        vox_normalise_vector(light.values, light.values);
    } else {
        light.values[0] = 0.0f;
        light.values[1] = 0.0f;
        light.values[2] = 1.0f;
    }

    VOX_Matrix camera;
    compute_camera(&light, &camera);
    g_VEHDRAW_CAVIAR.light_vector() = camera_position;

    VOX_Matrix inverse_camera;
    vox_matrix_inverse(camera.values, inverse_camera.values);
    vox_matrix_vector_mul(inverse_camera.values, camera_position.values,
                          vector.values);
    g_VEHDRAW_CAVIAR.shadow_vector() = vector;
    g_VEHDRAW_CAVIAR.set_shadow_table(ShadowColorTable);

    vector.values[0] = camera.values[2][0] * -10000.0f;
    vector.values[1] = camera.values[2][1] * -10000.0f;
    vector.values[2] = camera.values[2][2] * -10000.0f;
    g_VEHDRAW_CAVIAR.set_scaling(scale_numerator * 0.07f / scale_denominator);
    g_VEHDRAW_CAVIAR.set_camera_direct(&vector, &camera);

    vehdraw_construct_vehicle(vehicle_id, &g_VEHDRAW_CAVIAR);
    const int rotation = (frame - 3) & 7;
    g_VEHDRAW_CAVIAR.set_scene_rotation(
        static_cast<float>(rotation) * 0.785375f
            - VehDrawCameraAngles.values[0],
        0.0f, 0.0f);

    // The old test is Buffer+0x50 == 0. Expose that already-typed state by
    // a read-only Buffer API rather than retaining the absolute address.
    if (!g_VEHDRAW_BUFFER.has_locked_bits()) {
        g_VEHDRAW_BUFFER.init(200, 200, 0, 0);
        g_VEHDRAW_BUFFER.sync_to_palette(&g_PALETTE1);
    }

    const unsigned int render_flags =
        ((original_flags & 0x2000) | 0x1000) >> 12;
    g_VEHDRAW_CAVIAR.render(buffer, x, y, render_flags);

    // Rendering may mutate the selected vehicle's prototype; reload it after
    // the render, matching the original order.
    const int proto_id = Vehs[vehicle_id].proto_id;
    if (VehPrototypes[proto_id].armor_id == 5) {
        g_VEHDRAW_CAVIAR.reset_objects();
        vehdraw_construct_armor(Vehs[vehicle_id].faction_id,
                                VehPrototypes[proto_id].chassis_id,
                                VehPrototypes[proto_id].weapon_id,
                                5, 1, &g_VEHDRAW_CAVIAR);
        const unsigned int destination_flags = (original_flags >> 12) & 2;
        g_VEHDRAW_CAVIAR.render_multi_table_dest(
            buffer, x, y, 0x18,
            ArmorTransparentColorTables,
            destination_flags);
        return;
    }
    g_VEHDRAW_CAVIAR.clear_buffers(0);
}


// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004C1B10 ?vehdraw_compute_facet_normals@@YAXXZ 0x004C1B10-0x004C1CF7
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original control flow and original seed extent; observed VC6 external symbol. Tests exercise actual bodies, call/publication order, unordered/zero camera branches and complete facet tables at both established baselines.", "Camera publishes its first Euler triple before the second rotation; original CRT atan calls round their quotient to double. Facet initialization snapshots ConsoleGlobal inherited MapWin field at1DD6C, established by receiver-relative MapWin initialization; four independent 2401-element tables replace cross-object pointer arithmetic."], "reason": "Vehicle rendering setup with real VOX types, source-owned tables and actual Console terrain-height state."}
void __cdecl vehdraw_compute_facet_normals() {
    // 0x93341C is ConsoleGlobal + MapWin::field_1DD6C_, initialized to 25
    // or 8 by MapWin::calculate_dimensions. It is not an independent global.
    const int baseline = (int)ConsoleGlobal.field_1DD6C_;
    int index = 0;
    for (int i = 30; i < 240; i += 30) {
        const float height_i = (float)((i >> 2) - baseline);
        for (int j = 30; j < 240; j += 30) {
            const float height_j = (float)((j >> 2) - baseline);
            for (int k = 30; k < 240; k += 30) {
                const float height_k = (float)((k >> 2) - baseline);
                for (int l = 30; l < 240; l += 30) {
                    const float height_l = (float)((l >> 2) - baseline);
                    VOX_Vect a = {{0.0f, 50.0f, height_j}};
                    VOX_Vect b = {{-50.0f, 0.0f, height_i}};
                    VOX_Vect c = {{50.0f, 0.0f, height_k}};
                    VOX_Vect d = {{0.0f, -50.0f, height_l}};
                    VOX_Vect n0, n1, n2, n3;
                    vox_cross_product(a.values, b.values, n0.values);
                    vox_cross_product(c.values, a.values, n1.values);
                    vox_cross_product(d.values, c.values, n2.values);
                    vox_cross_product(b.values, d.values, n3.values);
                    vox_normalise_vector(n0.values, n0.values);
                    vox_normalise_vector(n1.values, n1.values);
                    vox_normalise_vector(n2.values, n2.values);
                    vox_normalise_vector(n3.values, n3.values);
                    VehDrawFacetNormals[0][index] = n0;
                    VehDrawFacetNormals[1][index] = n1;
                    VehDrawFacetNormals[2][index] = n2;
                    VehDrawFacetNormals[3][index] = n3;
                    ++index;
                }
            }
        }
    }
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x004C1D00 vehdraw_nullsub1 0x004C1D00-0x004C1D01 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x004C1D00-0x004C1D01 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x004C1B10 ?vehdraw_compute_facet_normals@@YAXXZ (src/vehicle_render.cpp) and 0x004C1D10 ?vehdraw_setup@@YAXXZ (src/vehicle_render.cpp)"]}
// symbol    ?vehdraw_nullsub1@@YAXXZ
// kind      game
void __cdecl vehdraw_nullsub1() {
}

// ORIGINAL: 0x004C1D10 ?vehdraw_setup@@YAXXZ 0x004C1D10-0x004C1D15 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original4C1D10 is a five-byte tail jump to4C1B10; load_voxels451066 calls this entry. No extra renderer initialization or copied thunk bytes."], "reason": "Recover the voxel-loader setup entry as an ordinary call to the existing vehicle facet-normal owner."}

void __cdecl vehdraw_setup() {
    vehdraw_compute_facet_normals();
}


// ORIGINAL: 0x004C1D20 ?vehdraw_construct_vehicle@@YAHHPAUCaviar@@@Z 0x004C1D20-0x004C202D
// RECOVERY_BINDING: v1 {"evidence": ["Original resets renderer count before scaling and reloads prototype identity after morale calculation. Ordinary fields are byte-zero-extended, armor defense uses signed comparison, and faction comes from the current vehicle. Native special prototypes 8/9/10 clamp morale before scaling and choose their actual animated model.", "The repeated ordinary final reactor selection is expressed through the genuine shared reactor operation. Catalogued integer return is not consumed by mapped callers; original exits expose incidental register contents, not a supported result."], "reason": "Recover vehicle model assembly from genuine vehicle/prototype state and loaded CaviarData owners."}
// symbol    ?vehdraw_construct_vehicle@@YAXHPAVCaviar@@@Z
void __cdecl vehdraw_construct_vehicle(int veh_id, Caviar *renderer) {
    renderer->reset_objects();
    int proto_id = Vehs[veh_id].proto_id;
    const int initial_chassis = VehPrototypes[proto_id].chassis_id;
    if (initial_chassis == 0) {
        const int weapon = VehPrototypes[proto_id].weapon_id;
        renderer->set_scaling(renderer->get_scaling() *
            ((weapon < 3 || weapon == 21) ? 1.25f : 0.8f));
    } else if (initial_chassis == 1 && VehPrototypes[proto_id].weapon_id == 21) {
        renderer->set_scaling(renderer->get_scaling() * 1.25f);
    }
    int morale;
    if (Vehs[veh_id].faction_id != 0) morale = morale_veh(veh_id, 0, 0);
    else morale = static_cast<int>(morale_alien(veh_id, ConsoleGlobal.faction_id_));
    // Morale calculation may change the vehicle/prototype state.
    proto_id = Vehs[veh_id].proto_id;
    if (proto_id == 8 || proto_id == 9 || proto_id == 10) {
        const int maximum = proto_id == 10 ? 7 : 6;
        if (morale < 0) morale = 0;
        else if (morale > maximum) morale = maximum;
        const int base = proto_id == 8 ? 12 : (proto_id == 9 ? 6 : 5);
        renderer->set_scaling(static_cast<float>(
            static_cast<double>(renderer->get_scaling()) * (morale + base) * 0.125f));
        if (proto_id == 8)
            renderer->add_object(&g_NW_CAVIARDATA, VehDrawAnimationStep % 5 + 30);
        else if (proto_id == 9)
            renderer->add_object(&g_NI_CAVIARDATA, VehDrawAnimationStep % 5 + 30);
        else
            renderer->add_object(&g_NLC_CAVIARDATA, VehDrawAnimationStep);
        return;
    }
    const uint32_t abilities = VehPrototypes[proto_id].ability_flags;
    const int reactor = VehPrototypes[proto_id].reactor_id;
    const int weapon = VehPrototypes[proto_id].weapon_id;
    int chassis = VehPrototypes[proto_id].chassis_id;
    const int armor = VehPrototypes[proto_id].armor_id;
    const int faction_id = Vehs[veh_id].faction_id;
    renderer->reset_objects();
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    vehdraw_construct_chassis(faction_id, chassis, weapon, armor, reactor, renderer);
    vehdraw_construct_weapon(faction_id, chassis, weapon, renderer);
    vehdraw_construct_abil(abilities, renderer, faction_id);
    vehdraw_construct_armor(faction_id, chassis, weapon, armor, 0, renderer);
    vehdraw_construct_reactor(faction_id, chassis, weapon, reactor, abilities, renderer);
}


// ORIGINAL: 0x004C2030 ?vehdraw_construct_abil@@YAHHPAUCaviar@@H@Z 0x004C2030-0x004C2360 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original type15 bonus loop calls the technology predicate with signed short prerequisite, reloads bonus value after that call, uses the direct 32-bit variable shift whose count masking is implicit in the selected x86 instruction, and reloads loop count on continuation. Bonus additions and the subsequent ordinary ability additions are not deduplicated.", "Original faction alien flag selects human or alien colony/terraforming attachments; remaining masks select independently loaded model owners. Mapped callers discard the catalogued integer result.", "Pinned 0x004C20A7 shifts the reloaded ability bonus id directly; an explicit source &31 adds a non-original instruction. Keep the preceding Ability lookup, has_tech call, subsequent bonus-id reload and repeated model additions intact."], "reason": "Recover ordered ability model additions from actual faction bonuses and rule records, retaining repeated selections."}
// symbol    ?vehdraw_construct_abil@@YAXIPAVCaviar@@H@Z
void __cdecl vehdraw_construct_abil(uint32_t abilities, Caviar *renderer, int faction_id) {
    for (int i = 0; i < Players[faction_id].faction_bonus_count; ++i) {
        if (Players[faction_id].faction_bonus_id[i] != RULE_FREEABIL) continue;
        if (!has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id))
            continue;
        // has_tech precedes the second read of the bonus index in the image.
        const uint32_t bonus = abilities &
            (1u << (Players[faction_id].faction_bonus_val1[i]));
        if (bonus & 0x1000000u) renderer->add_object(&g_SP_SOPORIFIC_CAVIARDATA, 0);
        if (bonus & 0x2000000u) renderer->add_object(&g_SP_DISSWAVE_CAVIARDATA, 0);
        if (bonus & 0x8000000u) renderer->add_object(&g_SP_NANOO_CAVIARDATA, 0);
        if (bonus & 0x4000000u) renderer->add_object(&g_SP_MARINED_CAVIARDATA, 0);
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (bonus & 1u) renderer->add_object(&g_SSFA_CAVIARDATA, 0);
            if (bonus & 0x2000u) renderer->add_object(&g_SFTA_CAVIARDATA, 0);
        } else {
            if (bonus & 1u) renderer->add_object(&g_SSF_CAVIARDATA, 0);
            if (bonus & 0x2000u) renderer->add_object(&g_SFT_CAVIARDATA, 0);
        }
        if (bonus & 0x20u) renderer->add_object(&g_SAS_CAVIARDATA, 0);
        if (bonus & 0x80u) renderer->add_object(&g_SCD_CAVIARDATA, 0);
        if (bonus & 0x400u) renderer->add_object(&g_SAGS_CAVIARDATA, 0);
        if (bonus & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA1, 0);
        if (bonus & 0x200000u) renderer->add_object(&g_SRB_CAVIARDATA, 0);
        if (bonus & 0x10u) renderer->add_object(&g_SDP_CAVIARDATA, 0);
        if (bonus & 0x200u) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
        if (bonus & 0x40000u) renderer->add_object(&g_SHT_CAVIARDATA, 0);
    }
    // Bonus parts and explicitly selected parts are not deduplicated.
    if (abilities & 0x1000000u) renderer->add_object(&g_SP_SOPORIFIC_CAVIARDATA, 0);
    if (abilities & 0x2000000u) renderer->add_object(&g_SP_DISSWAVE_CAVIARDATA, 0);
    if (abilities & 0x8000000u) renderer->add_object(&g_SP_NANOO_CAVIARDATA, 0);
    if (abilities & 0x4000000u) renderer->add_object(&g_SP_MARINED_CAVIARDATA, 0);
    if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
        if (abilities & 1u) renderer->add_object(&g_SSFA_CAVIARDATA, 0);
        if (abilities & 0x2000u) renderer->add_object(&g_SFTA_CAVIARDATA, 0);
    } else {
        if (abilities & 1u) renderer->add_object(&g_SSF_CAVIARDATA, 0);
        if (abilities & 0x2000u) renderer->add_object(&g_SFT_CAVIARDATA, 0);
    }
    if (abilities & 0x20u) renderer->add_object(&g_SAS_CAVIARDATA, 0);
    if (abilities & 0x80u) renderer->add_object(&g_SCD_CAVIARDATA, 0);
    if (abilities & 0x400u) renderer->add_object(&g_SAGS_CAVIARDATA, 0);
    if (abilities & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA1, 0);
    if (abilities & 0x200000u) renderer->add_object(&g_SRB_CAVIARDATA, 0);
    if (abilities & 0x10u) renderer->add_object(&g_SDP_CAVIARDATA, 0);
    if (abilities & 0x200u) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
    if (abilities & 0x40000u) renderer->add_object(&g_SHT_CAVIARDATA, 0);
}


// ORIGINAL: 0x004C2360 ?vehdraw_construct_vehicle@@YAHHHHHHHPAUCaviar@@@Z 0x004C2360-0x004C246D BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4C23BC loads argument6 as ability flags and pushes faction as the ability helper final argument. The seed swapped these inputs. Original signed armor-defense comparison may substitute infantry for a supply crawler.", "Reactor selection follows all component builders and therefore observes current weapon mode. Original void exits and all mapped callers support no returned value; the legacy catalogue name remains provenance."], "reason": "Recover explicit vehicle component assembly through real chassis, weapon, ability, armor and reactor operations."}
// symbol    ?vehdraw_construct_vehicle@@YAXHHHHHIPAVCaviar@@@Z
void __cdecl vehdraw_construct_vehicle(int faction_id, int chassis, int weapon,
                                      int armor, int reactor, uint32_t abilities,
                                      Caviar *renderer) {
    renderer->reset_objects();
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    vehdraw_construct_chassis(faction_id, chassis, weapon, armor, reactor, renderer);
    vehdraw_construct_weapon(faction_id, chassis, weapon, renderer);
    vehdraw_construct_abil(abilities, renderer, faction_id);
    vehdraw_construct_armor(faction_id, chassis, weapon, armor, 0, renderer);
    // This operation reads Weapon.mode after the component builders.
    vehdraw_construct_reactor(faction_id, chassis, weapon, reactor, abilities, renderer);
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C2470 0x004C2C88 0x004C2CC0 0x004C24A7
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C2470 0x004C2CC0 0x004C2CE4 0x004C25AD
// ORIGINAL: 0x004C2470 ?vehdraw_construct_chassis@@YAHHHHHHPAUCaviar@@@Z 0x004C2470-0x004C2C87
// RECOVERY_BINDING: v1 {"evidence": ["Original bounds faction only for chassis tint selection, uses signed armor defense, and selects real reactor-minus-one model arrays. Utility infantry falls through the shared add at4C27AA and also adds VLIGHTS at4C27B8, omitted by the seed.", "Original rover steps and aircraft faction flags are reloaded at call boundaries. Crawler assembly uses signed step remainder then queries the genuine DesignWin Win base at4C2BE9 and increments the current animation storage."], "reason": "Recover chassis model ownership, tint selection and animation through genuine renderer operations."}
// symbol    ?vehdraw_construct_chassis@@YAXHHHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_chassis(int faction_id, int chassis, int weapon,
                                      int armor, int reactor, Caviar *renderer) {
    if (faction_id < 0 || faction_id > 7) faction_id = 0;
    unsigned char *body_colors = &BodyTintColorTables[faction_id][0][0];
    unsigned char *armor_colors = 0;
    int armor_grade = 0;
    switch (armor) {
    case 1: case 5: armor_grade = 1; break;
    case 2: case 10: case 11: armor_grade = 1; armor_colors = &ArmorTintColorTables[0][0][0]; break;
    case 3: armor_grade = 1; armor_colors = &ArmorTintColorTables[1][0][0]; break;
    case 6: case 12: case 13: armor_grade = 1; armor_colors = &ArmorTintColorTables[2][0][0]; break;
    case 7: armor_grade = 1; armor_colors = &ArmorTintColorTables[3][0][0]; break;
    case 4: case 8: armor_grade = 2; break;
    }
    if (armor == 10 || armor == 12) renderer->add_object(&g_SCJ_CAVIARDATA, 0);
    if (armor == 11 || armor == 13) renderer->add_object(&g_SHT_CAVIARDATA, 0);
    if (weapon == 21 && chassis == 1 && Armor[armor].defense_rating <= 1) chassis = 0;
    if (chassis < 0 || chassis > 8) return;

    if ((chassis == 1 || chassis == 3 || chassis == 5 || chassis == 6 || chassis == 7)
        && reactor != 0) {
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            if (armor_colors)
                renderer->add_object(&g_ACP_CAVIARDATA[reactor - 1], armor_colors, 0);
            else renderer->add_object(&g_ACP_CAVIARDATA[reactor - 1], 0);
        } else renderer->add_object(&g_VRC_CAVIARDATA[reactor - 1], 0);
    }
    switch (chassis) {
    case 0:
        if (Weapon[weapon].mode < 3) {
            if (weapon == 17) return;
            CaviarData *model = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AX_CAVIARDATA : &g_VI_CAVIARDATA;
            renderer->add_object(model, &VehInfantryOffset, VehDrawAnimationStep % 15);
        } else if (weapon == 21) {
            if (Players[faction_id].rule_flags & RFLAG_ALIEN)
                renderer->add_object(&g_AX_CAVIARDATA, &ArmorTintColorTables[3][0][0],
                                     &VehInfantryOffset, VehDrawAnimationStep % 15);
            else renderer->add_object(&g_VPT_CAVIARDATA, &VehInfantryOffset,
                                      VehDrawAnimationStep % 15);
        } else if (weapon != 22 &&
                   (!(Players[faction_id].rule_flags & RFLAG_ALIEN) || weapon != 17)) {
            renderer->add_object(&g_VB_CAVIARDATA, 0);
            renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCL_CAVIARDATA, 0);
            renderer->add_object(&g_VCLT00_CAVIARDATA, 0);
            renderer->add_object(&g_VLIGHTS_CAVIARDATA, 0);
        }
        return;
    case 1: case 3:
        renderer->add_object(&g_VGMC_CAVIARDATA, 0);
        renderer->add_object(&g_VGMCP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
        if (chassis == 3) { renderer->add_object(&g_VFL_CAVIARDATA, 0); return; }
        renderer->add_object(&g_VSP_CAVIARDATA, 0);
        renderer->add_object(&g_VSPTB_CAVIARDATA, VehDrawAnimationStep);
        renderer->add_object(&g_VSPTF_CAVIARDATA, VehDrawAnimationStep);
        renderer->add_object(&g_VLIGHTS_CAVIARDATA, 0);
        return;
    case 2:
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VHT_VBP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VHTP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VHTTP_CAVIARDATA, body_colors, 0);
        if (armor_grade == 0) {
            renderer->add_object(&g_VHTA0_CAVIARDATA[0], 0);
            renderer->add_object(&g_VHTTPA0_CAVIARDATA[0], 0);
        }
        renderer->add_object(&g_VLIGHTS_CAVIARDATA, &VehHoverLightsOffset, 0);
        return;
    case 4: case 5: case 6: case 7:
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
        renderer->add_object(&g_VGMC_CAVIARDATA, 0);
        renderer->add_object(&g_VGMCP_CAVIARDATA, body_colors, 0);
        if (chassis == 4) {
            renderer->add_object(&g_VCU_CAVIARDATA, 0);
            renderer->add_object(&g_VCUP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCUW_CAVIARDATA, 0);
            if (armor_grade == 0) renderer->add_object(&g_VCUA0_CAVIARDATA[0], 0);
        } else if (chassis == 5) {
            renderer->add_object(&g_VJTP_CAVIARDATA, body_colors, 0);
            if (armor_grade == 0)
                renderer->add_object((Players[faction_id].rule_flags & RFLAG_ALIEN)
                    ? &g_AA_WING_CAVIARDATA[0] : &g_VJT0_CAVIARDATA[0], 0);
        } else if (chassis == 6) {
            renderer->add_object(&g_VCT_CAVIARDATA, 0);
            renderer->add_object(&g_VCTP_CAVIARDATA, body_colors, 0);
            renderer->add_object(&g_VCTB_CAVIARDATA, VehDrawAnimationStep % 15);
            if (TutWinDesWindow.is_visible())
                VehDrawAnimationStep = static_cast<int>(static_cast<uint32_t>(VehDrawAnimationStep) + 1u);
            if (armor_grade == 0) renderer->add_object(&g_VCT0_CAVIARDATA[0], body_colors, 0);
        } else {
            renderer->add_object(&g_VGS_CAVIARDATA, 0);
            renderer->add_object(&g_VGSP_CAVIARDATA, body_colors, 0);
        }
        return;
    case 8:
        if (weapon == 16) renderer->add_object(&g_VM13_CAVIARDATA, 0);
        else {
            renderer->add_object(&g_VM_CAVIARDATA, 0);
            if (weapon == 24) renderer->add_object(&g_TECLOAD_CAVIARDATA, 0);
            else if (weapon == 25) renderer->add_object(&g_FUNLOAD_CAVIARDATA, 0);
        }
        renderer->add_object(&g_VB_CAVIARDATA, 0);
        renderer->add_object(&g_VBP_CAVIARDATA, body_colors, 0);
    }
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C2CF0 0x004C306C 0x004C3084 0x004C2F0F
// ORIGINAL: 0x004C2CF0 ?vehdraw_construct_weapon@@YAHHHHPAUCaviar@@@Z 0x004C2CF0-0x004C306A
// RECOVERY_BINDING: v1 {"evidence": ["Original allocates and initializes a26-entry automatic model lookup, not static storage. Offense minus1 and99 have distinct models; bounded other values include a no-model sentinel. Special weapons/chassis return before ordinary faction/rule work.", "Original terraform handling rereads faction flags after the first model addition. Gun, hover-gun and drop offsets are the actual vectors populated by load_voxels, not old executable addresses."], "reason": "Recover weapon model selection and attachments through real model arrays, tint tables and vector offsets."}
// symbol    ?vehdraw_construct_weapon@@YAXHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_weapon(int faction_id, int chassis, int weapon, Caviar *renderer) {
    const int model_indices[26] = {-1,-1,0,0,1,3,2,2,4,4,5,5,6,7,7,7,8,8,8,8,9,9,9,9,10,13};
    if (chassis == 8) return;
    if (weapon == 16) { renderer->add_object(&g_VM13_CAVIARDATA, 0); return; }
    if (weapon == 23 || weapon == 24 || weapon == 25) {
        renderer->add_object(&g_VM_CAVIARDATA, 0);
        if (weapon == 24) renderer->add_object(&g_TECLOAD_CAVIARDATA, 0);
        else if (weapon == 25) renderer->add_object(&g_FUNLOAD_CAVIARDATA, 0);
        return;
    }
    int offense = Weapon[weapon].offense_rating;
    if (faction_id < 0 || faction_id > 7) faction_id = 0;
    unsigned char *body_colors = &BodyTintColorTables[faction_id][0][0];
    const int mode = Weapon[weapon].mode;
    if (mode < 3) {
        int model;
        if (offense == -1) model = 11;
        else if (offense == 99) model = 12;
        else {
            if (offense < 0) offense = 0;
            else if (offense > 25) offense = 25;
            model = model_indices[offense];
        }
        if (model != -1) {
            CaviarData *definition = weapon == 12 ? &g_RESLASER_CAVIARDATA : &g_VW_CAVIARDATA[model];
            if (chassis == 0) {
                renderer->add_object(definition, &VehGunMountOffset, 0);
                renderer->add_object(&g_VGMT_CAVIARDATA, 0);
                renderer->add_object(&g_VGMTP_CAVIARDATA, body_colors, 0);
            } else if (chassis == 2) renderer->add_object(definition, &VehHoverGunOffset, 0);
            else renderer->add_object(definition, 0);
        }
    } else {
        switch (mode) {
        case 7: renderer->add_object(&g_VWNTT_CAVIARDATA, 0); break;
        case 8:
            if (chassis != 0) renderer->add_object(&g_DROPLET_CAVIARDATA, &VehDropOffset, 0);
            else if (Players[faction_id].rule_flags & RFLAG_ALIEN)
                renderer->add_object(&g_ACOLPOD_CAVIARDATA, VehDrawAnimationStep % 15);
            else {
                renderer->add_object(&g_DROP_CAVIARDATA, 0);
                renderer->add_object(&g_DROPLET_CAVIARDATA, 0);
            }
            break;
        case 9:
            if (chassis == 0)
                renderer->add_object((Players[faction_id].rule_flags & RFLAG_ALIEN)
                    ? &g_VTA_CAVIARDATA : &g_VT_CAVIARDATA, 0);
            // The first add can change faction state; this is a fresh test.
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                renderer->add_object(&g_AT_CAVIARDATA, 0);
                renderer->add_object(&g_ATP_CAVIARDATA, 0);
            } else renderer->add_object(&g_VWNTU_CAVIARDATA, 0);
            break;
        case 10: renderer->add_object(&g_VWNST_CAVIARDATA, 0); break;
        case 11: if (chassis != 0) renderer->add_object(&g_PTMOD_CAVIARDATA, 0); break;
        case 12: renderer->add_object(&g_VWNAA_CAVIARDATA, 0); break;
        }
    }
    if (weapon == 13) {
        if (chassis == 0) renderer->add_object(&g_RESBOLT_CAVIARDATA, &VehGunMountOffset, 0);
        else if (chassis == 2) renderer->add_object(&g_RESBOLT_CAVIARDATA, &VehHoverGunOffset, 0);
        else renderer->add_object(&g_RESBOLT_CAVIARDATA, 0);
    }
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C3090 0x004C3798 0x004C37D0 0x004C30AE
// RECOVERY_ASSOCIATED: v1 switch_targets 0x004C3090 0x004C37D0 0x004C37F4 0x004C3257
// ORIGINAL: 0x004C3090 ?vehdraw_construct_armor@@YAHHHHHHPAUCaviar@@@Z 0x004C3090-0x004C3795
// RECOVERY_BINDING: v1 {"evidence": ["Original special and ordinary armor cases are separate; transparent armor5 uses the first constant-color ramp. Reactor-independent model arrays are indexed from their actual starts rather than biased original address expressions.", "Original crawler armor adds its crawler model followed by the human/alien body armor, with the same selected tint on both. Do not clamp faction here: unlike chassis and weapon, the original does not."], "reason": "Recover armor overlays, tint and ordered body-specific pairs using actual model owners."}
// symbol    ?vehdraw_construct_armor@@YAXHHHHHPAVCaviar@@@Z
void __cdecl vehdraw_construct_armor(int faction_id, int chassis, int weapon,
                                    int armor, int transparent, Caviar *renderer) {
    bool special = false;
    bool ordinary = false;
    unsigned char *colors = 0;
    switch (armor) {
    case 1: ordinary = true; break;
    case 2: case 10: case 11: ordinary = true; colors = &ArmorTintColorTables[0][0][0]; break;
    case 3: ordinary = true; colors = &ArmorTintColorTables[1][0][0]; break;
    case 6: case 12: case 13: ordinary = true; colors = &ArmorTintColorTables[2][0][0]; break;
    case 7: ordinary = true; colors = &ArmorTintColorTables[3][0][0]; break;
    case 5:
        if (transparent) { ordinary = true; colors = &ConstantColorTables[0][0]; }
        else special = true;
        break;
    case 4: case 8: case 9:
        if (chassis == 0 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
            CaviarData *model = armor == 4 ? &g_VIPTAWL_CAVIARDATA :
                (armor == 8 ? &g_VIPTASGN_CAVIARDATA : &g_VIPTAPSI_CAVIARDATA);
            renderer->add_object(model, &VehInfantryOffset, VehDrawAnimationStep);
        } else {
            renderer->add_object(&g_A_CAVIARDATA, 0);
            renderer->add_object(armor == 4 ? &g_APWALL_CAVIARDATA :
                (armor == 8 ? &g_ASGEN_CAVIARDATA : &g_APSID_CAVIARDATA), 0);
        }
        special = true;
        break;
    }
    CaviarData *first = 0;
    CaviarData *second = 0;
    switch (chassis) {
    case 0:
        if (special || !ordinary) return;
        if (weapon != -1 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
            first = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AA_CAVIARDATA : &g_VIPTA_CAVIARDATA[0];
            if (colors) renderer->add_object(first, colors, &VehInfantryOffset, VehDrawAnimationStep % 15);
            else renderer->add_object(first, &VehInfantryOffset, VehDrawAnimationStep % 15);
            return;
        }
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 1:
        if (special || !ordinary) return;
        if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
            first = &g_AA01_CAVIARDATA; second = &g_AA_ROVER_CAVIARDATA;
        } else { first = &g_VA01_CAVIARDATA; second = &g_VSPA0_CAVIARDATA[0]; }
        break;
    case 2:
        if (special) { first = &g_VHTA0_CAVIARDATA[0]; second = &g_VHTTPA0_CAVIARDATA[0]; }
        else if (ordinary) { first = &g_VHTA0_CAVIARDATA[1]; second = &g_VHTTPA0_CAVIARDATA[1]; }
        break;
    case 3:
        if (special || !ordinary) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 4:
        if (special) {
            if (armor != 5) first = &g_VCUA0_CAVIARDATA[0];
        } else if (ordinary) first = &g_VCUA0_CAVIARDATA[1];
        break;
    case 5:
        if (special) {
            if (armor != 5) first = (Players[faction_id].rule_flags & RFLAG_ALIEN)
                ? &g_AA_WING_CAVIARDATA[0] : &g_VJT0_CAVIARDATA[0];
        } else if (ordinary) {
            if (Players[faction_id].rule_flags & RFLAG_ALIEN) {
                first = &g_AA01_CAVIARDATA; second = &g_AA_WING_CAVIARDATA[1];
            } else { first = &g_VA01_CAVIARDATA; second = &g_VJT0_CAVIARDATA[1]; }
        }
        break;
    case 6:
        if (special) {
            if (armor == 5) return;
            first = &g_VCT0_CAVIARDATA[0];
        } else {
            if (!ordinary) return;
            first = &g_VCT0_CAVIARDATA[1];
        }
        second = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case 7:
        if (!ordinary) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    case -1:
        if (!ordinary || special) return;
        first = (Players[faction_id].rule_flags & RFLAG_ALIEN) ? &g_AA01_CAVIARDATA : &g_VA01_CAVIARDATA;
        break;
    }
    if (first) {
        if (colors) renderer->add_object(first, colors, 0);
        else renderer->add_object(first, 0);
    }
    if (second) {
        if (colors) renderer->add_object(second, colors, 0);
        else renderer->add_object(second, 0);
    }
}


// ORIGINAL: 0x004C3800 ?vehdraw_construct_reactor@@YAHHHHHHPAUCaviar@@@Z 0x004C3800-0x004C3896 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original returns before array access when reactor is zero. VIPTR, VPBR0 and VR operands are biased by one12-byte model and therefore mean reactor-minus-one into existing arrays.", "Infantry and chassis8 return after their selected model; only the general reactor case tests ability0x8000 and adds VHR2. Original4C387D reloads flags before exit, so seed returned add_object values were invented."], "reason": "Recover reactor attachments through source-owned model arrays and infantry placement."}
// symbol    ?vehdraw_construct_reactor@@YAXHHHHIPAVCaviar@@@Z
void __cdecl vehdraw_construct_reactor(int faction_id, int chassis, int weapon,
                                      int reactor, uint32_t abilities, Caviar *renderer) {
    UNUSED(faction_id);
    if (reactor == 0) return;
    if (chassis == 0 && (Weapon[weapon].mode < 3 || Weapon[weapon].mode == 11)) {
        renderer->add_object(&g_VIPTR_CAVIARDATA[reactor - 1], &VehInfantryOffset, 0);
        return;
    }
    if (chassis == 8) {
        renderer->add_object(&g_VPBR0_CAVIARDATA[reactor - 1], 0);
        return;
    }
    renderer->add_object(&g_VR_CAVIARDATA[reactor - 1], 0);
    if (abilities & 0x8000u) renderer->add_object(&g_VHR_CAVIARDATA2, 0);
}

// Unreferenced in the pinned image (no call, jump or data reference).
// Original effect: return without modifying EAX; callee stack cleanup 0 bytes.
// ORIGINAL: 0x004C38A0 vehdraw_nullsub2 0x004C38A0-0x004C38A1 BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason": "Bind the unreferenced original leaf as an empty/constant source function in the translation unit whose address band contains it; its original name, prototype and callers remain unknown.", "evidence": ["pinned:terranx_original.exe:0x004C38A0-0x004C38A1 return without modifying EAX; callee stack cleanup 0 bytes", "pinned scan: no rel32 call/jmp and no dword reference to this address anywhere in the image", "address band: nearest mapped neighbours are 0x004C3800 ?vehdraw_construct_reactor@@YAHHHHHHPAUCaviar@@@Z (src/vehicle_render.cpp) and 0x004C38B0 _$E21 (src/worldwin.cpp); 0x004C38B0 is the worldwin.cpp file-start dynamic initializer, so this is the last function of the vehicle-rendering band"]}
// symbol    ?vehdraw_nullsub2@@YAXXZ
// kind      game
void __cdecl vehdraw_nullsub2() {
}


