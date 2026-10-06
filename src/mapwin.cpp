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

// MapWin constructor-installed Win branch: capture4, resize13, mouse17, update63.
// Table start is observed in the original constructor; full extent remains unknown.
// RECOVERY_GENERATED: v1 vtable 0x0066A57C ? ??_7MapWin@@6BWin@@@ 0x004626E0
#include "original_seam.h"
#include "mapwin.h"
#include "lock.h"      // map_tile_bytes() - terrain_fixup walks its records
#include "console.h"
#include "planwin.h"   // PlanWindow is the complete MapWin-derived singleton.
#include "diplopop.h"  // PlanWin receiver checks popup visibility.
#include "tutwin.h"    // TutWinGlobal - draw_cursor ends on its draw_arrow
#include "statuswin.h"
#include "worldwin.h"
#include "spritebox.h"
#include "net_class.h"
#include "texture.h"
#include "sprite.h"    // g_TER1_* band: gen_terrain_poly's sprite receivers
#include "random.h"    // Random local_rand - gen_terrain_poly's per-tile roll
#include "path.h"      // Paths.get and the route-preview pathfinder.
#include "general.h"   // range - veh.h's inline needs it (already below)
#include "game.h"      // ShadowColorTable/TerritoryColorTables/state globals
#include "veh.h"       // Vehs/VehPrototypes - the visibility walk
#include "base.h"      // Bases/base_at/crop_yield
#include "faction.h"   // PlayersData - visibility and treaty reads
#include "infowin.h"   // InfoFactionTextColors - the territory fill colours
#include "imagebutton.h"
#include "basewin.h"
#include "map.h"       // MapLongitudeBounds / MapLatitudeBounds / MapIsFlat / map_tiles
#include "general.h"   // mem_get, and the game-state flags the painters read
#include "strings.h"   // StringTemp is also the shared main-caption scratch buffer.
#include "temp.h"      // PlanLineCounter controls the patterned-line phase.
#include "alpha.h"     // Rules movement rate and label_get text lookup.
#include "pulldown.h"  // The context menu owns its live item and submenu rows.
#include "xpops.h"     // right_menu confirms the original whole-stack order.
#include "popup.h"     // click owns a genuine stack Popup for base routing.
#include "netmsg.h"    // NetMsgPrimary reports invalid inter-base routes.
#include "netdaemon.h" // click owns the vehicle/base lock interval.
#include "time.h"      // go_reset clears the prior capture/cursor state.
#include <cstring>
#include <stdlib.h>

// Eight pointer slots, initially zero; startup publishes slot0 at0x0058F1C1
// after constructing slots1..7. draw_tile bounds its walk at0x007D3C5C.
// RECOVERY_OBJECT: v2 0x007D3C3C 0x007D3C5C ?MapWinTable@@3PAPAVMapWin@@A 0x0058F040
MapWinSlots MapWinTable;

// The original startup pass writes all 256 dwords immediately after the
// map-window slots (with one intervening dword). Its consumers and meaning
// remain to be identified; keep the storage distinct from MapWinTable.
// RECOVERY_OBJECT: v2 0x007D3C60 0x007D4060 ?MapWinStartupWords@@3PAHA 0x00471030
int MapWinStartupWords[256];

// The original layout writer MapWin_UNK5 and mode-zero initializer share
// these two 16-byte records and their accumulated dirty bits.
// RECOVERY_OBJECT: v2 0x00939F48 0x00939F68 ?MapPanelRects@@3PAUMapPanelRect@@A 0x0046FA20
MapPanelRect MapPanelRects[2];
// RECOVERY_OBJECT: v2 0x00939EDC 0x00939EE0 ?MapPanelDirtyMask@@3IA 0x0046FA20
uint32_t MapPanelDirtyMask;

// 0x007D3C28..0x007D3C38 - rectangle published by calculate_dimensions for
// the primary window: west/north/east/south map-view bounds, also consumed by
// MapWin::focus's base-window hit test.
// Original462A9F..462AEB publishes four adjacent dwords;46B330 passes their
// start to in_box. The loader supplies sixteen zero bytes, and both compiler
// flag sets emit a standalone tagRECT object. Do not absorb the next dword.
// RECOVERY_OBJECT: v2 0x007D3C28 0x007D3C38 ?MapWinPrimaryCorner@@3UtagRECT@@A 0x00462980
RECT MapWinPrimaryCorner;

void __fastcall MapWin_UNK5(MapWin *window, void *, int panel);
int __cdecl veh_find(int x, int y, int faction_id, int skip_vehicle_id);
int __cdecl veh_ready(int vehicle_id);
void __cdecl help_any();
void __cdecl action_arty(int vehicle_id, int x, int y);
void __cdecl action_patrol(int vehicle_id, int x, int y);

namespace {
int mapwin_dest_add(int left, int right);
int mapwin_dest_sub(int left, int right);
int mapwin_dest_mul(int left, int right);
}

// The active map bounds, panel layouts, faction caption records and tutorial
// base rectangle all have source-owned receivers below.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00470A90 ?init@MapWin@@QAEXHH@Z 0x00470A90-0x00470EC2
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00470a90.cpp retains the complete 1074-byte seed; mode zero uses MapWin_UNK5 panels and Players records, mode two uses BaseWin::rect1_, and mode three selects label 0x204.", "src/recovered/units/00588aa0.cpp stores find_font(10, 1) at the original display-font slot."], "reason": "Home the map modes through typed source owners; resolve the font through its real lookup while its startup cache remains unowned."}
void MapWin::init(int mode, int update) {
    buffer_a_.init(100, 50, 0, 0);
    field_1DD74_ = 1;
    RECT bounds;
    if (this == MapWinTable[1]) {
        bounds = WorldMapBounds;
        field_1DD98_ = -8;
    } else if (mode == 0) {
        unsigned int index = field_1DD78_;
        const int panel_count = IsMultiplayerNet ? 2 : 1;
        if (index < static_cast<unsigned int>(panel_count)) {
            if ((MapPanelDirtyMask & (1U << index)) == 0) {
                MapWin_UNK5(this, 0, index);
            }
            bounds.left = MapPanelRects[index].left;
            bounds.top = MapPanelRects[index].top;
            bounds.right = MapPanelRects[index].right;
            bounds.bottom = MapPanelRects[index].bottom;
            if (index != 0) field_1DD98_ = -8;
        } else {
            bounds.left = static_cast<int>(index * 0x28);
            bounds.top = bounds.left;
            bounds.right = bounds.left + 0x190;
            bounds.bottom = bounds.top + 0x190;
        }
    } else if (mode == 2) {
        bounds = TutWinBaseWindow.rect1_;
    } else {
        bounds = WorldMapBounds;
    }

    StringTemp[0] = '\0';
    if (mode == 0) {
        strcat(StringTemp, label_get(0x1D));
        if (field_1DD78_ == 0) {
            StringTemp[0] = '\0';
            const Player &player = Players[ConsoleGlobal.faction_id_];
            GenderDefault = player.noun_gender;
            PluralityDefault = player.is_noun_plural;
            strcat(StringTemp, player.formal_name_faction);
            strcat(StringTemp, ": ");
            strcat(StringTemp, label_get(0));
            strcat(StringTemp, " ");
            say_year(StringTemp);
        } else {
            strcat(StringTemp, " ");
            strcat(StringTemp, "#");
            char number[80];
            _itoa(static_cast<int>(field_1DD78_) + 1, number, 10);
            strcat(StringTemp, number);
        }
    } else {
        strcat(StringTemp, label_get(mode == 3 ? 0x204 : 0x1E));
    }

    int flags = mode == 3 ? 0x400020 : 0x20000430;
    if (mode == 0 && field_1DD78_ != 0) flags = 0x20010430;
    if (this == &WorldClimateWorldWin.mapWin_) {
        flags = (flags & ~0x10) | 0x40400001;
    }
    init(&bounds, StringTemp, flags);
    if (mode == 0) {
        field_1DD70_ = 0x10EE3;
        field_1DD98_ = field_1DD78_ != 0 ? -8 : 0;
    } else {
        field_1DD70_ = 0x80000082;
        field_1DD98_ = -7;
    }
    if (field_1DD78_ != 0) {
        // The original font-table initializer caches find_font(10, 1) at
        // 0x00945778. Resolve the same Font through its typed owner until
        // that initializer and cache have source-owned lifetimes.
        Win::field_F8_ = find_font(10, 1);
    }
    if (mode == 0) {
        do_image_buttons();
        field_1DD70_ &= 0x9FFFFFFF;
        if (field_1DD78_ == 0) {
            field_1DD70_ |= IsMultiplayerNet ? 0x20000000 : 0x60000000;
        } else {
            if (field_1DD78_ == 1) field_1DD70_ |= 0x40000000;
            standard_caption();
        }
        if (update == 0) {
            if (WorldClimateSkipTerrainClear != 0) {
                field_1DD70_ &= 0xFFFFF73F;
            }
            redraw();
            field_1DD70_ |= 0x8C0;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00470920 ?init@MapWin@@QAEXPAURECT@@PADH@Z 0x00470920-0x00470A90
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00470920.cpp; original calls clear, cache setup and GraphicWin::init(RECT*) for the PlanWin, secondary MapWin slot, WorldWin and ordinary receivers.", "Original 0x004709EB identifies the complete PlanWin at 0x00834D70 and 0x004709F3 tests DiplomacyPopup; 0x005D50A0 supplies null-RECT behavior."], "reason": "Home receiver selection through real PlanWin, popup, interface and WorldWin owners and the original GraphicWin rectangle overload."}
// symbol    ?init@MapWin@@QAEXPAUtagRECT@@PADH@Z
void MapWin::init(RECT *bounds, char *caption, int flags) {
    clear(field_1DD78_);
    init_dummy();
    field_1DD74_ = 1;

    if ((field_1DD70_ & 0x1000000U) != 0 || this == MapWinTable[0]) {
        return;
    }
    if (this == static_cast<MapWin *>(&PlanWindow)) {
        // Both original visibility paths pass the same parent and style;
        // the first returns directly and the second joins the common tail.
        DiplomacyPopup.is_visible();
        GraphicWin::init(bounds, caption, flags, &MainInterfaceGlobal, 0, 0);
        return;
    }
    if (this == MapWinTable[1]) {
        GraphicWin::init(bounds, 0, 0x20, &MainInterfaceGlobal, 0, 0);
        return;
    }
    if (this == &WorldClimateWorldWin.mapWin_) {
        GraphicWin::init(bounds, 0, 0, 0, 0, 0);
        return;
    }
    GraphicWin::init(bounds, caption, flags | 0x30000000, 0, 0, 0);
}

// 0x006874C4: draw_my_dest publishes a vehicle index after validation;
// StatusWin clears it to -1. Pinned original .data bytes at 0x006874C4 are
// FF FF FF FF; the initializer preserves that startup sentinel. The typed
// source owner below records the corresponding four-byte original interval.
// RECOVERY_OBJECT: v2 0x006874C4 0x006874C8 ?DestinationVehicleId@@3HA 0x004B4BF0
int DestinationVehicleId = -1;

// 0x0093A940 gates base-overlay visibility alongside ControlTurnPhase.
// Pinned PE section evidence places exactly this four-byte read in the
// zero-fill tail of .data, initialized to 0. Console::human_turn (0x514085)
// enables it before selecting vehicles for the player-turn loop.
// RECOVERY_OBJECT: v2 0x0093A940 0x0093A944 ?MapBaseDisplayOverride@@3HA 0x00467190
int MapBaseDisplayOverride;

void __cdecl base_draw(Buffer *target, int base_id, int pixel_x, int pixel_y,
                       int style, int flags);

void __cdecl draw_my_dest(int vehicle_id);

// The farm display and terrain-fixup pass share one complete MapWin. Its
// same-type heap allocation at 0x58F177 requests 0x22480 bytes; the constructor's
// GraphicWin base at +0x21A6C writes through +0xA13, independently closing
// that extent. All original image-initial bytes are zero.
// CRT[0x682540] invokes 0x462080, constructing this most-derived object and
// registering 0x4620A0. That exit destroys the MapWin own part (0x420F90),
// then its GraphicWin virtual base (0x5D4DD0), both from receiver 0x7F5ACC.
// C++ owns this complete lifetime; runtime init_dummy/cache population and
// shutdown are separate, still-unfinished startup dependencies.
// RECOVERY_OBJECT: v2 0x007D4060 0x007F64E0 ?FarmMapWindow@@3VMapWin@@A 0x00462080
MapWin FarmMapWindow;

// Four coordinate pairs: terrain copies eight DWORDs at464D12..464D21 and
// draw_map translates four Vert objects at46A940..46A95D. The following
// 7D3C28 rectangle is a separate object, not a fifth corner.
// RECOVERY_OBJECT: v2 0x007D3BE8 0x007D3C08 ?MapCornerCopy@@3PAUVert@@A 0x0046A550
Vert MapCornerCopy[4];
// Terrain's four-corner producer and BaseWin's farm consumer share this buffer.
// RECOVERY_OBJECT: v2 0x007D3C08 0x007D3C28 ?MapCornerScratch@@3PAUVert@@A 0x004632D0
Vert MapCornerScratch[4];
// Original46AB95/46ABDF call the actual USER32 rectangle predicate.
// RECOVERY_IMPORT: v1 __imp__IsRectEmpty@4 USER32.dll name:IsRectEmpty

// Initially zero; main_game's still-unfinished startup enables map input.
// RECOVERY_OBJECT: v2 0x0090D938 0x0090D93C ?MapInputEnabled@@3HA 0x0046F000
int MapInputEnabled;
// All three original references are the mouse handler's pump-reentry guard.
// RECOVERY_OBJECT: v2 0x007F67C4 0x007F67C8 ?MapMouseFeedbackBusy@@3HA 0x0046F000
int MapMouseFeedbackBusy;
// The route builder resets/accumulates this separately from corner storage.
// RECOVERY_OBJECT: v2 0x007D3B70 0x007D3B74 ?MapDestinationCost@@3HA 0x00467430
int MapDestinationCost;

// RECOVERY_OBJECT: v2 0x00945AE0 0x00945AE4 ?MapEditorPaintId@@3HA 0x0046B510
int MapEditorPaintId;
// RECOVERY_OBJECT: v2 0x00945AE4 0x00945AE8 ?MapEditorBrushId@@3HA 0x0046B510
int MapEditorBrushId;

struct EditorTerrainRule { uint32_t paint; uint32_t erase_first; };
static const uint32_t remove_mine_or_solar = BIT_SENSOR_ARRAY |
    BIT_THERMAL_BORE | BIT_ECH_MIRROR | BIT_CONDENSER | BIT_FOREST |
    BIT_SOLAR_TIDAL | BIT_MINE;
// Tools 6..25: source-level terrain rules corresponding to the first two
// words of each original 0x20-byte UI descriptor. The remaining descriptor
// words are presentation data, not read by either recovered caller.
static const EditorTerrainRule EditorTerrainRules[20] = {
    {BIT_FARM, BIT_FOREST},
    {BIT_SOIL_ENRICHER, BIT_FOREST},
    {BIT_MINE, remove_mine_or_solar},
    {BIT_SOLAR_TIDAL, remove_mine_or_solar},
    {BIT_FOREST, BIT_THERMAL_BORE | BIT_ECH_MIRROR | BIT_CONDENSER |
                 BIT_FOREST | BIT_SOIL_ENRICHER | BIT_FARM |
                 BIT_SOLAR_TIDAL | BIT_FUNGUS | BIT_MINE},
    {BIT_ROAD, 0},
    {BIT_MAGTUBE, 0},
    {BIT_BUNKER, BIT_AIRBASE},
    {BIT_AIRBASE, BIT_BUNKER},
    {BIT_SENSOR_ARRAY, BIT_THERMAL_BORE | BIT_ECH_MIRROR | BIT_CONDENSER |
                       BIT_SOLAR_TIDAL | BIT_MINE},
    {0, BIT_FUNGUS},
    {BIT_FUNGUS, BIT_FOREST | BIT_SOIL_ENRICHER | BIT_FARM |
                 BIT_SOLAR_TIDAL | BIT_MINE},
    {BIT_CONDENSER, remove_mine_or_solar},
    {BIT_ECH_MIRROR, remove_mine_or_solar},
    {BIT_THERMAL_BORE, remove_mine_or_solar | BIT_FARM},
    {0, 0}, {0, 0}, {0, 0}, {0, 0},
    {BIT_MONOLITH, BIT_SUPPLY_POD | BIT_NUTRIENT_RSC | BIT_RSC_BONUS},
};

uint32_t map_editor_paint_mask(int tool_id) {
    return tool_id >= 6 && tool_id < 26
        ? EditorTerrainRules[tool_id - 6].paint : 0;
}

uint32_t map_editor_erase_mask(int tool_id) {
    return tool_id >= 6 && tool_id < 26
        ? EditorTerrainRules[tool_id - 6].erase_first : 0;
}

// Original46B510's retained seed supplies the control flow and tool cases.
// The tile and redraw receivers now use their source-owned C++ objects.
// ORIGINAL: 0x0046B510 ?editor@MapWin@@QAEXHHH@Z 0x0046B510-0x0046BFCF
// RECOVERY_BINDING: v1 {"evidence": ["Original46F000 calls46B510; git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0046b510.cpp retains the complete seed and original bounds.", "Map tile, MapWin receiver, radius, faction and base uses resolve through typed source declarations; original 0x00691888 tool descriptor words 0/1 are modeled as named terrain rules shared with Console."], "reason": "Implement the original map-editing paths and shared editor selection in source-owned C++ while retaining the seed and original identity; this is coverage work, not a measured matching claim."}
// TRIED: `mov edx,edx` prologue zeroing (used explicit `bool`/int locals instead of a single xor'd register reused for 0); divergence is in the prologue register setup (push vs xor at insn #4), not in the switch body, which was cross-checked field-for-field against src/map.h's Map struct and the 11-entry jump table.
void MapWin::editor(int tile_x, int tile_y, int erase) {
    if (field_1DD8C_ != 0) {
        if (tile_x == static_cast<int>(field_1DD90_) &&
            tile_y == static_cast<int>(field_1DD94_)) return;
        field_1DD90_ = tile_x;
        field_1DD94_ = tile_y;
    }

    int step = MapEditorBrushId == 1 ? 2 : 1;
    int radius = MapEditorBrushId;
    if (radius > 1) --radius;
    int limit = RadiusRange[radius];
    int first = 0;
    uint32_t river_changed = 1;
    int changed = 0;
    bool stop = false;
    bool used_outer_ring = false;
    int width = MapLongitudeBounds;

    for (int pass = 0; pass <= 8 && !stop;) {
        if (pass != 0) {
            if (MapEditorBrushId != 0 || field_1DD8C_ != 0) break;
            first = RadiusRange[pass - 1];
            limit = RadiusRange[pass];
            step = 1;
            used_outer_ring = true;
        }
        for (int index = first; index < limit && !stop;
             index += index == 0 ? 1 : step) {
            int x = tile_x + RadiusOffsetX[index];
            if ((MapIsFlat & 1) == 0) {
                if (x < 0) x += width;
                else if (x >= width) x -= width;
            }
            const int y = tile_y + RadiusOffsetY[index];
            if (x < 0 || x >= width || y < 0 || y >= MapLatitudeBounds)
                continue;

            Map &tile = map_tiles()[MapLongitude * y + (x >> 1)];
            Map &origin = map_tiles()[MapLongitude * tile_y + (tile_x >> 1)];
            switch (MapEditorPaintId) {
            case 0:
                if (field_1DD8C_ != 0) {
                    if (erase != 0) {
                        if ((tile.climate & 0xE0) >= 0x60)
                            world_alt_set(x, y, 3, 1);
                    } else if ((tile.climate & 0xE0) < 0x60) {
                        world_alt_set(x, y, 2, 1);
                    }
                } else if (erase != 0) {
                    world_lower_alt(x, y);
                } else {
                    world_raise_alt(x, y);
                }
                changed = 1;
                break;
            case 1: {
                const int rocky = tile.val3 >> 6;
                const int next = erase ? (rocky > 0 ? rocky - 1 : 0)
                                       : (rocky < 2 ? rocky + 1 : 2);
                rocky_set(x, y, next);
                if (next != rocky) changed = 1;
                break;
            }
            case 2:
                river_changed = erase ? origin.bit & BIT_RIVERBED
                                      : (~origin.bit >> 8) & 1;
                bit_set(tile_x, tile_y, BIT_RIVERBED, erase == 0);
                stop = true;
                break;
            case 3:
                if (erase != 0) {
                    bit_set(tile_x, tile_y, BIT_RSC_BONUS | BIT_NUTRIENT_RSC, 0);
                } else if ((origin.bit & BIT_RSC_BONUS) == 0) {
                    bit_set(tile_x, tile_y, BIT_RSC_BONUS | BIT_NUTRIENT_RSC, 1);
                    bit_set(tile_x, tile_y, BIT_SUPPLY_POD, 0);
                } else if ((origin.bit & BIT_ENERGY_RSC) == 0) {
                    if ((origin.bit & BIT_MINERAL_RSC) == 0)
                        bit_set(tile_x, tile_y, BIT_ENERGY_RSC, 1);
                    else bit_set(tile_x, tile_y, BIT_MINERAL_RSC, 0);
                } else {
                    bit_set(tile_x, tile_y, BIT_ENERGY_RSC, 0);
                    bit_set(tile_x, tile_y, BIT_MINERAL_RSC, 1);
                }
                stop = true;
                break;
            case 4:
                if (erase == 0) {
                    if ((tile.bit & BIT_SUPPLY_REMOVE) != 0 ||
                        (tile.bit & BIT_SUPPLY_POD) == 0) changed = 1;
                    bit_set(x, y, BIT_SUPPLY_POD, 1);
                    bit_set(x, y, BIT_SUPPLY_REMOVE | BIT_NUTRIENT_RSC |
                                      BIT_RSC_BONUS, 0);
                } else {
                    if ((tile.bit & BIT_SUPPLY_POD) != 0 ||
                        (tile.bit & BIT_SUPPLY_REMOVE) == 0) changed = 1;
                    bit_set(x, y, BIT_SUPPLY_POD, 0);
                    bit_set(x, y, BIT_SUPPLY_REMOVE, 1);
                }
                break;
            case 5: {
                const int faction = ConsoleGlobal.faction_id_;
                const unsigned char visibility = 1 << faction;
                if (erase == 0) {
                    if ((PlayersData[faction].flags & PFLAG_MAP_REVEALED) == 0 &&
                        (tile.visibility & visibility) == 0) changed = 1;
                    spot_loc(x, y, faction);
                    synch_bit(x, y, faction);
                } else {
                    if ((PlayersData[faction].flags & PFLAG_MAP_REVEALED) != 0 ||
                        (tile.visibility & visibility) != 0) changed = 1;
                    tile.visibility &= ~visibility;
                    const int base_id = base_at(x, y);
                    if (base_id >= 0) Bases[base_id].visibility &= ~visibility;
                }
                break;
            }
            case 26:
                changed = (tile.bit2 & 0x80000004u) == 4 &&
                          (tile.climate & 0xE0) >= 0x60;
                bit2_set(x, y, BIT2_JUNGLE, 0);
                break;
            case 27:
            case 28:
            case 29: {
                const uint32_t flag = MapEditorPaintId == 27 ? BIT2_DUNES :
                    MapEditorPaintId == 28 ? BIT2_URANIUM : BIT2_GEOTHERMAL;
                changed = ((tile.bit2 & (0x80000000u | flag)) == flag) ==
                          (erase != 0);
                bit2_set(x, y, flag, erase == 0);
                if (erase == 0) bit2_set(x, y, BIT2_UNK_80000000, 0);
                break;
            }
            default: {
                const int tool = MapEditorPaintId;
                if (tool < 6 || tool >= 26) break;
                uint32_t mask = map_editor_paint_mask(tool);
                const bool same_side = ((tile.climate & 0xE0) < 0x60) ==
                    ((origin.climate & 0xE0) < 0x60);
                if (!same_side) break;
                if (erase == 0) {
                    if ((tile.bit & mask) == 0)
                        changed = (mask & 0xC) != 0 ? 2 : 1;
                    if ((mask & BIT_MAGTUBE) != 0) mask |= BIT_ROAD;
                    bit_set(x, y, map_editor_erase_mask(tool), 0);
                    bit_set(x, y, mask, 1);
                } else {
                    if ((tile.bit & mask) != 0) changed = 1;
                    bit_set(x, y, mask, 0);
                }
                break;
            }
            }
            for (int faction = 1; faction < 8; ++faction)
                synch_bit(x, y, faction);
            width = MapLongitudeBounds;
        }
        ++pass;
        if (changed != 0) break;
    }

    if (field_1DD8C_ != 0 && (field_1DD8C_ & 4) == 0) {
        draw_radius(tile_x, tile_y, radius, 2);
        return;
    }
    const int tool = MapEditorPaintId;
    if ((tool == 0 || tool == 2) && river_changed == 0) return;
    const bool climate_changed = tool == 0 || tool == 2 ||
                                 (tool == 0x12 && changed != 0);
    if (climate_changed) {
        world_climate();
    }
    if (climate_changed || used_outer_ring || MapEditorBrushId != 0 ||
        changed > 1) {
        for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *window = MapWinTable[slot];
            if (window && (slot == 0 || window->field_1DD74_ != 0))
                window->draw_map(1);
        }
    } else if (changed != 0) {
        for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
            MapWin *window = MapWinTable[slot];
            if (window && (slot == 0 || window->field_1DD74_ != 0))
                window->draw_radius(tile_x, tile_y, 0, 2);
        }
    }
}

// ORIGINAL: 0x0046C020 ?UNK4@MapWin@@QAEXHH@Z 0x0046C020-0x0046C0C9 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:b20012dad66f767e0e5f05b7aea45b88bd63a09d:src/unrecovered/0046c020.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations. Superseded scaffold retired at user request; historical claims and seed findings are transferred, not freshly granted. Current matching/restoration and any documented unfinished stages remain obligations."}
// kind      game
// LEVER: table walk over 0x7D3C3C..0x7D3C5C needed its own `extern MapWin *g_7d3c3c_tbl[]` (pointer-increment do/while, `p == g_7d3c3c_tbl` for the first-element special case) instead of the scaffold's fixed-address `int *const g_007d3c3c`.
void MapWin::UNK4(int tile_x, int tile_y) {
    editor(tile_x, tile_y, static_cast<int>(field_1DD8C_ & 2));
    field_1DD8C_ = 0;

    if (MapEditorPaintId == 0 || MapEditorPaintId == 2 ||
        MapEditorPaintId == 0x12) {
        world_climate();
    }

    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *const window = MapWinTable[slot];
        if (window != nullptr &&
            (slot == 0 || window->field_1DD74_ != 0)) {
            window->draw_map(1);
        }
    }
}

/*
Purpose: Build a map window - attach its embedded GraphicWin virtual base,
         then its own TextureStore/Buffer/Font/ImageButton members.
// ORIGINAL: 0x004626E0 ??0MapWin@@QAE@H@Z 0x004626E0-0x00462868;0x00655860-0x00655920 BYTE_EXACT
// symbol    ??0MapWin@@QAE@XZ
// size      584 bytes
// prototype void (__thiscall ??0MapWin@@QAE@H@Z)(MapWin* this, int)
// callers   3   call targets   9
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005D4CF0 0x006252A0 0x005D7210 0x00618EA0 0x006456E4 0x006457C2
Return Value: this

The compiler owns the construction machinery from the declarations in
mapwin.h: the vbtable store, the virtual GraphicWin base at +0x21A6C under
the most-derived flag, the two TextureStore vector ranges, the lone
TextureStore, the three Buffers, the three Fonts, the ImageButton array, and
the vbase-branch vtable pair the old hand body spelled at graphic_win and
graphic_win+0x444. The body keeps the eight scalar clears, which the image
runs after the members.
*/
MapWin::MapWin() {
    field_1DD74_ = 0;
    owned_ = 0;
    field_1DD80_ = 0;
    field_1DD8C_ = 0;
    field_21A44_ = 0;
    cursor_sprite_ = 0;
    field_1EEC0_ = static_cast<uint32_t>(0xffffd8f1);
    field_1EEC4_ = 0xffffffff;
}

/*
Purpose: Tear down a map window's own members. Nothing here calls
         MapWin::clear as MapWin - it is reached by name.
// ORIGINAL: 0x00420F90 ??1MapWin@@QAE@XZ 0x00420F90-0x004210CB;0x00651190-0x00651250 BYTE_EXACT
// TRIED: the three Buffer teardowns are the only divergence - the image makes
//            three `call ??1Buffer` (0x005D7410) where the in-class
//            `MEASURED virtual ~Buffer() { close(); }` in buffer.h expands
//            into each of them, and the expansion is what carries this body's
//            `sub esp, 8` for the spare slot, the `mov dword ptr [edi], 0`
//            vtable-null store and the `call 0x421088` unwind funclet - 93
//            instructions against the image's 73. Defining ~Buffer out of
//            line in buffer.cpp made THIS body byte-exact, 73 of 73, and
//            silenced the compiler's own deleting-destructor trio in every
//            other TU: deleting_thunks.cpp's ??_GBuffer claim at 0x005E3610
//            went UNRESOLVED, and that trade is not this file's to make.
//            Reverted; recorded here because the fix is one move away and
//            belongs to whoever owns both buffer.h and deleting_thunks.cpp.
// symbol    ??1MapWin@@UAE@XZ
// body      src/mapwin.h
// size      315 bytes
// prototype void (__thiscall ??1MapWin@@QAE@XZ)(MapWin* this)
// callers   1   call targets   6
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462870 0x005D7410 0x00618EE0 0x006252B0 0x006456E4
Return Value: n/a

The original receiver is the GraphicWin virtual-base subobject, not MapWin's
front. FarmMapWindow's registered exit at 0x4620A0 passes 0x7F5ACC (its front
plus 0x21A6C) first to this own-part destructor and then to GraphicWin's
destructor at 0x5D4DD0. Complete destruction therefore includes both stages.

The in-class destructor in mapwin.h owns only clear(0) explicitly. The
compiler supplies the EH frame, vtable transitions, vtordisp and member
destruction, emitting the own-part symbol identified above. A complete
object's generated destruction also tears down its GraphicWin virtual base;
do not reproduce the original receiver adjustment or double-destroy that base
by hand. The funclet span 0x00651190-0x00651250 belongs to the EH machinery.
*/
/*
Purpose: Unknown; the legacy implementation is a bare return with no body.
// ORIGINAL: 0x00467960 ?UNK3@MapWin@@QAEXXZ 0x00467960-0x00467961 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?UNK3@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::UNK3() {
}

/*
Purpose: Refresh the map's image buttons; the legacy implementation is a bare
         return, so the buttons are never rebuilt here.
// ORIGINAL: 0x0046FB80 ?do_image_buttons@MapWin@@QAEXXZ 0x0046FB80-0x0046FB81 BYTE_EXACT
// size      1 bytes
// prototype void (__thiscall ?do_image_buttons@MapWin@@QAEXXZ)(MapWin* this)
// callers   2   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::do_image_buttons() {
}


// The hit-test rectangle at 0x007AF50C is this object's +0xCEC member;
// MapWin::main_caption also selects its complete receiver at 0x007AE820.
// RECOVERY_OBJECT: v1 0x007AE820 ?MainInterfaceGlobal@@3VMainInterface@@A
MainInterface MainInterfaceGlobal;  // 0x007AE820

/*
Purpose: Push the map's caption into the main interface's date field. The
         original acts on the interface and caption it keeps at fixed
         addresses rather than on anything reached through the map window.
// ORIGINAL: 0x0046FB10 ?main_caption@MapWin@@QAEXXZ 0x0046FB10-0x0046FB20 BYTE_EXACT
// size      16 bytes
// prototype void (__thiscall ?main_caption@MapWin@@QAEXXZ)(MapWin* this)
// callers   11   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0045BE80
Return Value: n/a
Status: Complete
*/
void MapWin::main_caption() {
    // The original pushes the same scratch storage used by the string
    // producers, not an independent caption allocation.
    MainInterfaceGlobal.set_date(StringTemp);
}


/*
Purpose: Build the map-window caption from the selected label and publish it
         through the window's real GraphicWin/Win base.
*/
// ORIGINAL: 0x0046FB20 ?standard_caption@MapWin@@QAEXXZ 0x0046FB20-0x0046FB7B BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Retained original 0x0046FB20 seed; MapWin mode field and virtual GraphicWin/Win base; Labels and StringTemp source owners"], "reason": "Home the map window caption in the typed MapWin owner translation unit"}
void MapWin::standard_caption() {
    StringTemp[0] = '\0';
    int text_id;
    if ((field_1DD70_ & 0x40000000U) != 0) {
        text_id = reinterpret_cast<int *>(Labels.strings_ptr)[0x100];
    } else {
        text_id = reinterpret_cast<int *>(Labels.strings_ptr)[0x101];
    }
    strcat(StringTemp, StringTable.get(text_id));

    Win::set_caption(StringTemp);
}


/*
Purpose: Close the map window - free the buffer it owns at 0x4, then close its
         graphic base. The base is located through the object's own vbtable,
         not a fixed offset, so an embedded MapWin closes its own base.
// ORIGINAL: 0x00470F70 ?close@MapWin@@QAEXXZ 0x00470F70-0x00470F98 BYTE_EXACT
// size      40 bytes
// prototype void (__thiscall ?close@MapWin@@QAEXXZ)(MapWin* this)
// callers   4   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4E40 0x00644EF2
Return Value: n/a
Status: Complete
*/
void MapWin::close() {
    if (owned_) {
        free(owned_);
        owned_ = nullptr;
    }
    // THE IDIOMATIC SPELLING, BY DIRECTION (2026-09-01): call the base's
    // close, no cast. This body's claim was demoted for it - and that is
    // the point. The compiled form of this line disagrees with the image's
    // 15-instruction dispatch, which means THIS TREE IS MISSING A
    // RELATIONSHIP the original's declarations had (candidate hypotheses on
    // the marker below). The hand vbtable walk the old body spelled went
    // byte-exact, but it was bad C++ that masked the gap; the mismatch is
    // the signal to explore, not a wall to transcribe.
    GraphicWin::close();
}

// The primary console occupies slot zero and is closed by its own owner.
// Shutdown closes only populated secondary windows, then the separately
// constructed farm window, whose owned buffer is released by MapWin::close.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004710E0 ?mapwin_system_shutdown@@YAXXZ 0x004710E0-0x0047115F
// RECOVERY_BINDING: v1 {"evidence": ["src/unrecovered/004710e0.cpp retains the original loop over MapWinTable at 0x007D3C3C..0x007D3C5C and the final receiver at 0x007D4060", "src/mapwin.cpp binds those receivers as MapWinTable and FarmMapWindow; MapWin::close releases owned_ before GraphicWin::close"], "reason": "Replace the executable forwarder with typed shutdown of active secondary map windows and the independent farm window; preserve the seed for restoration measurement."}
void __cdecl mapwin_system_shutdown() {
    for (size_t slot = 1; slot < MapWinTableSlots; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (window != nullptr && window->field_1DD74_ != 0) {
            window->close();
        }
    }
    FarmMapWindow.close();
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00471030 ?mapwin_system_init@@YAXH@Z 0x00471030-0x004710DF
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x00471030-0x004710DF fills 0x007D3C60..0x007D4060, calls compute_lighting_table, clears/reinitializes eight MapWinTable slots, then initializes the FarmMapWindow cache", "src/unrecovered/00471030.cpp retains the seed; src/mapwin.cpp owns the table, FarmMapWindow and each called method"], "reason": "Recover map-window system startup with typed slot and storage ownership; leave the startup-word table's wider meaning open until consumers are identified."}
void __cdecl mapwin_system_init(int reset_windows) {
    FarmMapWindow.compute_lighting_table();
    for (size_t word = 0; word < 256; ++word) {
        MapWinStartupWords[word] = word >= 10 && word <= 65 ? 1 : 0;
    }
    for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (window == nullptr) continue;
        if (reset_windows != 0) {
            window->clear(static_cast<int>(slot));
            window->field_1DD74_ = slot == 0;
            if (slot == 1) MapWinTable[1]->field_1DD74_ = 1;
        }
        if (slot == 0 || window->field_1DD74_ != 0) {
            window->init(0, 0);
        }
    }
    FarmMapWindow.init_dummy();
}

// An indirect caller can request another map view centered on a tile. Slot
// zero is the console; only an allocated, inactive secondary slot is reused.
// Superseded seed deleted; seed paths below are historical Git provenance.
// Historical seed observations (not fresh measurements or acceptance):
// TRIED: for(;;){...if(p>=end)return;} vs do/while(p<end) vs goto-found form - all three land the same `jae + jmp` where the original has a single `jl` (140B vs 141B, one instruction over)
// ORIGINAL: 0x00470FA0 sub_470fa0 0x00470FA0-0x0047102D
// RECOVERY_BINDING: v1 {"evidence": ["Pinned original 0x00470FA0-0x0047102D scans MapWinTable slots 1..7 for a nonnull receiver with field_1DD74_ clear, calls clear and init, writes its 0x1DD9C/0x1DDA0 center, then invokes Win virtual slot 1 with zero", "src/unrecovered/00470fa0.cpp retains the two-argument seed and its compiler trials; src/win.h identifies virtual slot 1 as show(int)"], "reason": "Recover activation of a free secondary map view using its real MapWin receiver and inherited virtual show operation, not an invented vtable class."}
// symbol    ?mapwin_activate_free_slot@@YAXHH@Z
void __cdecl mapwin_activate_free_slot(int tile_x, int tile_y) {
    for (size_t slot = 1; slot < MapWinTableSlots; ++slot) {
        MapWin *window = MapWinTable[slot];
        if (window == nullptr || window->field_1DD74_ != 0) continue;
        window->clear(window->field_1DD78_);
        window->init(0, 1);
        window->field_1DD9C_ = tile_x;
        window->field_1DDA0_ = tile_y;
        window->show(0);
        return;
    }
}

/*
Purpose: Answer WM_SYSCLOSE: clear the window's per-slot state words, then run
         close() - the same owned-pointer free and vbtable walk, inlined.
// ORIGINAL: 0x0046F880 ?on_sys_close@MapWin@@QAEXXZ 0x0046F880-0x0046F8C0 BYTE_EXACT
// LEVER: byte-exact on promotion from the 0046f880 archive unit. The Q
//   spelling enters with `this` on the GraphicWin virtual base, and the image
//   folds the guard and clear off the ENTERED this ([ecx - 0x3cf4],
//   [ecx - 0x3cf8]) while materializing the walked-back front in esi only for
//   the inlined close tail. Spell each flag access as a ONE-SHOT
//   `((MapWin *)((uint8_t *)this - 0x21A6C))->field...` conversion and VC6
//   folds it the same way; naming one `base` local up front materializes esi
//   at the top and puts the guard/store on [esi + 0x1dd78] instead
//   (TRIED: single base local for everything - 17 of 19, two addressing
//   divergences). The image has close() INLINED here - its two callees
//   (0x005D4E40 GraphicWin::close, 0x00644EF2 free) are the only calls - so
//   the body spells the free and the vbtable walk out rather than calling
//   close(), whose E8 would be an edge the image lacks.
// size      64 bytes
// prototype void (__thiscall ?on_sys_close@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   2
// kind      game
// flags     sp_ready;purged_ok
// calls     0x005D4E40 0x00644EF2
Return Value: n/a
Status: Complete
*/
void MapWin::on_sys_close() {
    if (from_graphic_base(this)->field_1DD78_ != 0) {
        from_graphic_base(this)->field_1DD74_ = 0;
        MapWin *const base = from_graphic_base(this);
        if (base->owned_ != nullptr) {
            free(base->owned_);
            base->owned_ = nullptr;
        }
        // The idiomatic spelling, by direction - see close() above.
        base->GraphicWin::close();
    }
}

// Original0x0046F8C0 resets the side panels before dispatching one of the four
// map-window buttons. The fourth button toggles the custom world-scale label.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0046F8C0 ?on_button_clicked@MapWin@@QAEXH@Z 0x0046F8C0-0x0046F9E6 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0046f8c0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// symbol    ?on_button_clicked@MapWin@@UAEXH@Z
void MapWin::on_button_clicked(int button) {
    ConsoleInfoWin.reset();
    ConsoleStatusWin.reset();
    switch (button) {
    case 0:
        if (field_1DD78_ != 0) {
            field_1DD74_ = 0;
            close();
        }
        break;
    case 1:
        ConsoleGlobal.zoom(0x203, static_cast<int>(field_1DD78_));
        break;
    case 2:
        ConsoleGlobal.zoom(0x204, static_cast<int>(field_1DD78_));
        break;
    case 3:
        if (field_1DD78_ != 0) {
            field_1DD70_ ^= 0x40000000U;
            StringTemp[0] = '\0';
            const int label = (field_1DD70_ & 0x40000000U) != 0
                ? 0x100 : 0x101;
            strcat(StringTemp, label_get(label));
            Win::set_caption(StringTemp);
        }
        break;
    default:
        break;
    }
}


/*
Purpose: Report a left click on the map, but only when map input is enabled.
         Reached through the GraphicWin virtual base, so `this` points there
         and is adjusted back 0x21A6C to the MapWin before dispatching.
// ORIGINAL: 0x0046EBA0 ?on_left_click@MapWin@@QAEXHH@Z 0x0046EBA0-0x0046EBC5 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_left_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046D5D0
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_click(int a1, int a2) {
    if (MapInputEnabled == 0) {
        return;
    }
    MapWin *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    base->click(a1, a2, 0);
}

/*
Purpose: Report a right click on the map, but only when map input is enabled.
         Same virtual-base adjustment as the left click.
// ORIGINAL: 0x0046EBE0 ?on_right_click@MapWin@@QAEXHH@Z 0x0046EBE0-0x0046EC05 BYTE_EXACT
// size      37 bytes
// prototype void (__thiscall ?on_right_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046D5D0
Return Value: n/a
Status: Complete
*/
void MapWin::on_right_click(int a, int b) {
    if (MapInputEnabled == 0) {
        return;
    }
    MapWin *const base = reinterpret_cast<MapWin *>(
        reinterpret_cast<uint8_t *>(this) - 0x21A6C);
    base->click(a, b, 1);
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x0046EBD0 ?on_left_double_click@MapWin@@QAEXHH@Z 0x0046EBD0-0x0046EBD3 BYTE_EXACT
// symbol    ?on_left_double_click@MapWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_double_click@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_double_click(int, int) {
}

/*
Purpose: Unknown; the legacy implementation is a constant return that returns.
// ORIGINAL: 0x0046EFF0 ?on_left_up@MapWin@@QAEXHH@Z 0x0046EFF0-0x0046EFF3 BYTE_EXACT
// symbol    ?on_left_up@MapWin@@UAEXHH@Z
// size      3 bytes
// prototype void (__thiscall ?on_left_up@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
void MapWin::on_left_up(int, int) {
}


/*
Purpose: Unknown; the legacy implementation is a constant return that returns 0.
// ORIGINAL: 0x00462970 ?UNK2@MapWin@@QAEHXZ 0x00462970-0x00462973 BYTE_EXACT
// size      3 bytes
// prototype int (__thiscall ?UNK2@MapWin@@QAEHXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 0, always
Status: Complete
*/
int MapWin::UNK2() {
    return 0;
}


/*
Purpose: Broadcast a single-tile redraw to every live map window. Walks the
         eight map-window slots at 0x007D3C3C, skips empty ones, and for every
         slot past the first also skips windows whose 0x1DD74 activity dword is
         clear - slot 0 is the primary map window and always draws. Each
         surviving window is handed the tile through MapWin::draw_radius with a
         radius argument of 0; ?draw_tiles@@YAXHHH@Z at 0x0046B140 is the same
         body with a 1 there, and that single byte is the whole difference.
         The original opens no exception frame; there is nothing to omit.

         Kept as a full transcription rather than sharing a helper with
         draw_tiles, even though the two bodies differ in one byte. The
         originals are two separately compiled 70-byte functions, and
         tools/mutate_and_verify.py only derives mutants from bodies carrying
         an `Original Offset:` comment - and skips one-line bodies outright.
         Folding the walk into a shared helper would therefore hide the slot
         walk, the null guard, the slot-0 exemption, the activity gate and the
         argument order from mutation testing entirely, leaving both functions
         with zero mutants. The duplication buys verification.
// ORIGINAL: 0x0046AF40 ?draw_tile@@YAXHHH@Z 0x0046AF40-0x0046AF86 BYTE_EXACT
// LEVER: `window->draw_radius(...)` (the METHOD) in place of
//        `(ORIGINAL(window)->*MapWinOriginalDrawRadius)(...)` (the seam
//        variable) - the image's own call site is a direct `call 0x46a2a0`,
//        which the method compiles to; the seam load compiles an indirect
//        call. Same lever already used by the free `draw_radius` below.
// size      70 bytes
// prototype void (__cdecl ?draw_tile@@YAXHHH@Z)(int xCoord, int yCoord, int drawType)
// callers   34   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a. EAX on return is path-dependent leftover (draw_radius's own
              residue, the zero flag read at 0x0046AF5F, or a stale value when
              the last slot is empty), not a computed result, so unlike
              GraphicWin::close there is no residue to preserve and the void
              return is faithful.
Status: Broadcast body complete; radius renderer dependencies remain unfinished.
*/
// BODY IN mapwin.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


/*
Purpose: The radius-1 sibling of draw_tile - the identical 70-byte broadcast,
         but each live map window redraws the tile together with its
         surrounding ring rather than the single tile. The only difference in
         the originals is `push 1` at 0x0046B16D against `push 0` at
         0x0046AF6D; prologue, slot walk, null test, slot-0 exemption,
         activity gate, argument order, loop bound and epilogue all match
         instruction for instruction. Transcribed in full for the mutation
         coverage reason recorded on draw_tile.
// ORIGINAL: 0x0046B140 ?draw_tiles@@YAXHHH@Z 0x0046B140-0x0046B186 BYTE_EXACT
// LEVER: same as draw_tile above - `window->draw_radius(...)` in place of
//        the `MapWinOriginalDrawRadius` seam dispatch.
// size      70 bytes
// prototype
// callers   10   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a; same path-dependent EAX leftover as draw_tile.
Status: Broadcast body complete; radius renderer dependencies remain unfinished.
*/
// BODY IN mapwin.h, as `MEASURED inline`: the image writes it out at
// some call sites and calls it at others, and a .cpp definition is only ever
// one of those. The marker stays here because that is where the catalogue
// reads it.


// No redirect adapter for either. The original entry convention is __cdecl with
// three stack arguments and a caller-cleaned stack, and there is no
// this-adjustment to undo (contrast ListBox's destructor, entered at
// this + 0x48), so DllMain points 0x0046AF40 and 0x0046B140 straight at the
// recovered functions - the same shape as spying (0x0055BC00) and find_font
// (0x005882F0).

/*
Purpose: Report whether this object IS the process-wide Console.

             xor eax,eax / cmp ecx,0x9156B0 / sete al / ret

         An identity comparison against a fixed address, normalised to 0 or 1
         by `sete`. It dereferences nothing, so there is no memory access to
         model - only the address itself.

         0x009156B0 is already bound and named: ConsoleGlobal in console.h. So
         this adds no NEW fixed-address binding, it reuses the one the Console
         recovery already declared, and the comparison is written against the
         name rather than the literal.
// ORIGINAL: 0x00462960 ?UNK1@MapWin@@QAEHXZ 0x00462960-0x0046296C BYTE_EXACT
// size      12 bytes
// prototype int (__thiscall ?UNK1@MapWin@@QAEHXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: 1 when this is ConsoleGlobal, 0 otherwise
Status: Complete
*/
int MapWin::UNK1() {
    return this == reinterpret_cast<MapWin *>(&ConsoleGlobal) ? 1 : 0;
}

// Original0x0046B310 tests the requested tile against the visible scroll
// window, clamps it to the map, and recentres only when it reaches an edge.
// The primary Console-derived map has an additional base-window hit region.
// ORIGINAL: 0x0046B310 ?focus@MapWin@@QAEHHH@Z 0x0046B310-0x0046B50C FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0046b310.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// TRIED: literal register-mirroring transcription (eax/ecx/edx/ebx/edi as plain int locals) of the singleton-fast-path vs. wrap/clamp slow-path split on `this == 0x9156b0`; the sign-extraction idiom `and 0x80000000; neg; sbb; and 2; add 2` reduced to `(field < 0) ? 4 : 2`. MISMATCH #4 push/xor in the prologue - compiles and the branch/merge structure follows the disasm.
int MapWin::focus(int x_coord, int y_coord) {
    int moved = 0;
    if (this == static_cast<MapWin *>(&ConsoleGlobal)) {
        if (in_box(x_coord, y_coord, &MapWinPrimaryCorner) != 0) {
            return 0;
        }
        if (field_1DD78_ == 0) {
            ConsoleGlobal.field_23BFC_ = 1;
            ConsoleGlobal.field_23BF8_ = ConsoleGlobal.cursor_active_ == 0;
        }
        field_21A44_ = 1;
        set_center(x_coord, y_coord, 1);
        field_21A44_ = 0;
        return 1;
    }

    const bool flat = (MapIsFlat & 1) != 0;
    const int left = field_1DDA4_;
    int right = mapwin_dest_sub(mapwin_dest_add(left,
        mapwin_dest_mul(field_1DDD4_, 2)), 2);
    if (!flat) {
        if (right >= 0) {
            if (right >= static_cast<int>(MapLongitudeBounds)) {
                right = mapwin_dest_sub(right, MapLongitudeBounds);
            }
        } else {
            right = mapwin_dest_add(right, MapLongitudeBounds);
        }
    }
    const int top = field_1DDA8_;
    const int bottom = mapwin_dest_sub(mapwin_dest_add(top,
        mapwin_dest_mul(field_1DDD8_, 2)), 2);
    int x = x_coord;
    int y = y_coord;
    if (!flat) {
        const int wrap_threshold = mapwin_dest_mul(field_1DDCC_, 2);
        if (x <= wrap_threshold &&
            left >= mapwin_dest_sub(MapLongitudeBounds, wrap_threshold)) {
            x = mapwin_dest_add(x, MapLongitudeBounds);
        }
    }

    const int step = static_cast<int32_t>(field_1DD70_) < 0 ? 4 : 2;
    if (field_1DDAC_ == 0) {
        if (flat) {
            const int maximum = mapwin_dest_sub(
                mapwin_dest_sub(MapLongitudeBounds, step), 1);
            if (x < step || maximum < step) {
                x = step;
            } else if (x > maximum) {
                x = maximum;
            }
        } else {
            const int maximum = mapwin_dest_sub(right, step);
            if (x < step || maximum < step) {
                x = step;
            } else if (x > maximum) {
                x = maximum;
            }
        }
        if (x & 1) --x;
        if (y_coord & 1) ++x;
        if (!flat) {
            if (x <= mapwin_dest_add(left, step) ||
                x >= mapwin_dest_sub(right, step)) {
                moved = 1;
            }
        }
    }

    if (field_1DDB0_ == 0) {
        const int maximum = mapwin_dest_sub(
            mapwin_dest_sub(MapLatitudeBounds, step), 1);
        if (y < step || maximum < step) {
            y = step;
        } else if (y > maximum) {
            y = maximum;
        }
        if (x & 1) --x;
        if (y & 1) ++x;
        if (y <= mapwin_dest_add(top, step) ||
            y >= mapwin_dest_sub(bottom, step)) {
            moved = 1;
        }
    }

    if (moved == 0) {
        return 0;
    }
    if (field_1DD78_ == 0) {
        ConsoleGlobal.field_23BFC_ = 1;
        ConsoleGlobal.field_23BF8_ = ConsoleGlobal.cursor_active_ == 0;
    }
    set_center(x, y, 1);
    return moved;
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0046B1F0 ?set_center@MapWin@@QAEXHHH@Z 0x0046B1F0-0x0046B303
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0046b1f0.cpp and original 0x0046B1F0-0x0046B303; Vehs+0xE is faction_id."], "reason": "Home map recentering with real receiver fields, destination vehicle state and redraw."}
void MapWin::set_center(int x, int y, int draw_type) {
    TutWinGlobal->reset();
    if ((x ^ y) & 1) {
        if (x & 1) {
            --x;
        }
        if (y & 1) {
            ++x;
        }
    }
    const int old_x = field_1DD9C_;
    const int old_y = field_1DDA0_;
    int wrapped = x;
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) {
            wrapped += MapLongitudeBounds;
        } else if (x >= MapLongitudeBounds) {
            wrapped -= MapLongitudeBounds;
        }
    }
    field_1DD9C_ = wrapped;
    field_1DDA0_ = y;

    int destination = -1;
    if (this == static_cast<MapWin *>(&ConsoleGlobal)) {
        destination = DestinationVehicleId;
        if (destination >= 0 &&
            (Vehs[destination].faction_id == ConsoleGlobal.faction_id_ ||
             (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)) &&
            !(GameDataStateStorage.fields_.game_more_preferences_ & 0x2000000)) {
            CoverWindow.fill(9);
        }
    }
    if (old_x != field_1DD9C_ || old_y != field_1DDA0_) {
        draw_map(draw_type);
        if (field_1DD78_ == 0) {
            WorldClimateWorldWin.follow_center(x, y);
        }
    }
    if (destination >= 0) {
        draw_my_dest(destination);
    }
}

// The binder recorded the formerly unfinished state; the tile walker is now
// source-owned. base_draw remains the explicit unfinished rendering callee.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00469B40 ?gen_bases_only@MapWin@@QAEXXZ 0x00469B40-0x00469C9F
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00469b40.cpp retains the complete tile-walk seed; MapWin fields at 0x1DDA4..0x1DDD8 and map_loc determine the source-owned traversal."], "reason": "Home the base-only tile traversal reached by WorldWin redraw; base_draw remains an explicitly unfinished rendering dependency."}
void MapWin::gen_bases_only() {
    const int rows = field_1DDD0_ + field_1DDD8_ + 1;
    for (int row = 0; row < rows; ++row) {
        const int tile_y = field_1DDA8_ + row;
        int tile_x = field_1DDA4_ + (row & 1);
        int count = (row & 1) == 0 ? field_1DDD4_ : field_1DDCC_;
        while (count-- > 0) {
            int wrapped_x = tile_x;
            if ((MapIsFlat & 1) == 0) {
                if (wrapped_x < 0) {
                    wrapped_x += MapLongitudeBounds;
                } else if (wrapped_x >= MapLongitudeBounds) {
                    wrapped_x -= MapLongitudeBounds;
                }
            }
            if (tile_y >= 0 && tile_y < MapLatitudeBounds &&
                wrapped_x >= 0 && wrapped_x < MapLongitudeBounds) {
                Map *tile = map_loc(wrapped_x, tile_y);
                if ((tile->bit & 1) != 0 && (tile->val2 & 0xF) < 8) {
                    int pixel_x;
                    int pixel_y;
                    tile_to_pixel(wrapped_x, tile_y, &pixel_x, &pixel_y);
                    const int faction = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW)
                                            ? -1 : ConsoleGlobal.faction_id_;
                    draw_bases(wrapped_x, tile_y, pixel_x,
                               pixel_y - field_1DDC0_, faction);
                }
            }
            tile_x += 2;
        }
    }
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00467190 ?draw_bases@MapWin@@QAEXHHHHH@Z 0x00467190-0x004672B8
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00467190.cpp complete 296-byte seed; pinned original 0x00467190-0x004672B8; BaseWin+0x40B0C owns focused base id."], "reason": "Home map base-overlay eligibility and typed Base/Map visibility checks; base_draw remains explicitly unfinished."}
void MapWin::draw_bases(int tile_x, int tile_y, int pixel_x, int pixel_y,
                        int faction_id) {
    if ((field_1DD70_ & 0x200080) == 0) {
        return;
    }
    if (MapBaseDisplayOverride == 0 && ControlTurnPhase == 0 &&
        (GameDataStateStorage.fields_.game_rules_ & 0x80) == 0) {
        return;
    }
    const int base_id = base_at(tile_x, tile_y);
    if (base_id < 0) {
        return;
    }
    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0) {
        if ((PlayersData[faction_id].flags & 0x200) == 0 &&
            (map_loc(tile_x, tile_y)->visibility & (1 << faction_id)) == 0) {
            return;
        }
        if (Bases[base_id].faction_id_current != ConsoleGlobal.faction_id_ &&
            (Bases[base_id].visibility &
             (1 << ConsoleGlobal.faction_id_)) == 0) {
            return;
        }
    }
    if ((field_1DD70_ & 0x200000) != 0 &&
        base_id != TutWinBaseWindow.focused_base_id()) {
        return;
    }
    base_draw(static_cast<Buffer *>(static_cast<GraphicWin *>(this)), base_id,
              pixel_x, pixel_y, field_1DD98_,
              (field_1DD70_ >> 18) & 8);
}


/*
// ORIGINAL: 0x0046B1D0 ?on_redraw@MapWin@@QAEXXZ 0x0046B1D0-0x0046B1E8 BYTE_EXACT
// symbol    ?on_redraw@MapWin@@UAEXXZ
// size      24 bytes
// prototype void (__thiscall ?on_redraw@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046A550
// TRIED 2026-09-26 (Win slot-12 on_redraw rename batch): the rename made
//            this a genuine Win slot12 override, so VC6 now enters it with
//            the GraphicWin-subobject receiver instead of the complete
//            object (-0x21A6C delta folded into every this-relative
//            displacement). The raw offsets were re-anchored for that
//            convention: the flag read is +4 and the draw_map receiver is
//            the unadjusted this, which emits the image's
//            `mov eax,[ecx-0x21a68] ... add ecx,0xfffde594; call 0x46a550`.
//            The pre-rename spellings (-0x21a68 read, -0x21a6c receiver)
//            measured BYTE_EXACT only while the function overrode nothing.
Status: Complete
*/
void MapWin::on_redraw() {
    if (*reinterpret_cast<int *>(reinterpret_cast<char *>(this) + 4)) {
        draw_map(1);
    }
}

/*
// ORIGINAL: 0x0046FA00 ?on_resize@MapWin@@QAEXHH@Z 0x0046FA00-0x0046FA1A BYTE_EXACT
// symbol    ?on_resize@MapWin@@UAEXHH@Z
// size      26 bytes
// prototype void (__thiscall ?on_resize@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     sp_ready;purged_ok
// calls     0x0046FB80
Status: Complete
*/
void MapWin::on_resize(int, int) {
    if ((field_1DD70_ & 0x80000000) == 0) {
        do_image_buttons();
    }
}

// Scroll the retained image, then regenerate the exposed tile regions.
// Tile damage uses inclusive Y and possibly wrapped X, unlike pixel RECTs.
// The original's meaningful domain has live window/buffer objects and map
// coordinates, dimensions and altitude sums representable as signed ints.
// ORIGINAL: 0x0046A550 ?draw_map@MapWin@@QAEXH@Z 0x0046A550-0x0046AD51
// RECOVERY_BINDING: v1 {"evidence": ["Original46A550..46AD51 and caller46B1D0; retired seed git:b55fcf8b:src/unrecovered/0046a550.cpp records this identity and exact span.", "Original46AC00..46ACCD establishes independent rectangle snapshots;46A940..46A95D translates four shared Vert values;46AD18/46AD42 dispatch through Win slots17/63.", "Original46A58D..46A5A7 snapshots old origin only for an existing incremental image; calculate_dimensions owns initialization on full rendering."], "reason": "MapWin owns this unadjusted-this map-scrolling controller; direct callers and its tile/damage members identify the owner. The original independently saves each tile region around two clipped generations, unlike the retained transcription seed."}
// TRIED (retired seed git:b55fcf8b:src/unrecovered/0046a550.cpp): prologue diverges at #6 (orig 'push edi' before 'xor edi,edi', rebuilt reorders them) regardless of where the vbase/mapBuf virtual-base pointer is computed (top of function vs. first use) or where 'int edi' is declared/initialised; body is otherwise a full instruction-level transcription (goto per original jcc, register-named locals, stack slots as vXX locals) reaching 319/702 common mnemonics.
void MapWin::draw_map(int draw_type) {
    RECT horizontal_pixels = {0, 0, 0, 0};
    RECT vertical_pixels = {0, 0, 0, 0};
    Buffer *const canvas = static_cast<Buffer *>(this);

    if (field_1DDAC_ != 0 || field_1DDB0_ != 0) {
        field_21A44_ = 0;
    }
    // Initial full rendering has no previous origin: the constructor clears
    // the flag, not these coordinates. calculate_dimensions does not enable
    // incremental mode, so only snapshot an existing cached image.
    int old_x = 0;
    int old_y = 0;
    if (field_21A44_ != 0) {
        old_x = field_1DDA4_;
        old_y = field_1DDA8_;
    }
    calculate_dimensions();

    if (field_21A44_ != 0) {
        const int new_x = field_1DDA4_;
        const int new_y = field_1DDA8_;
        if (old_x == new_x && old_y == new_y) {
            return;
        }

        const int tile_width = field_1DDCC_ + field_1DDD4_;
        const int tile_height = field_1DDD0_ + field_1DDD8_;
        const MapTileDamageRect empty = {0, 0, 0, 0};
        if (new_x == old_x) {
            horizontal_damage_ = empty;
        } else {
            int left;
            int right;
            if (new_x > old_x) {
                if (new_x - old_x > tile_width) {
                    left = new_x - 1;
                    right = MapLongitudeBounds + old_x + 1;
                } else {
                    left = tile_width + old_x - 2;
                    right = tile_width + new_x;
                }
            } else {
                if (old_x - new_x > tile_width) {
                    left = tile_width + old_x - 1;
                    right = MapLongitudeBounds + new_x + tile_width - 1;
                } else {
                    left = new_x;
                    right = old_x + 1;
                }
            }
            if ((MapIsFlat & 1) == 0) {
                if (left < 0) {
                    left += MapLongitudeBounds;
                } else if (left >= MapLongitudeBounds) {
                    left -= MapLongitudeBounds;
                }
                if (right < 0) {
                    right += MapLongitudeBounds;
                } else if (right >= MapLongitudeBounds) {
                    right -= MapLongitudeBounds;
                }
            }
            horizontal_damage_.left = left - 1;
            horizontal_damage_.top = new_y;
            horizontal_damage_.right = right + 1;
            horizontal_damage_.bottom = new_y + tile_height;
        }

        if (new_y == old_y) {
            vertical_damage_ = empty;
        } else {
            int left = new_x;
            int right = new_x + tile_width;
            if ((MapIsFlat & 1) == 0) {
                if (left < 0) {
                    left += MapLongitudeBounds;
                } else if (left >= MapLongitudeBounds) {
                    left -= MapLongitudeBounds;
                }
                if (right < 0) {
                    right += MapLongitudeBounds;
                } else if (right >= MapLongitudeBounds) {
                    right -= MapLongitudeBounds;
                }
            }
            vertical_damage_.left = left;
            vertical_damage_.right = right;
            if (new_y > old_y) {
                vertical_damage_.top = tile_height + old_y - 1 - 4;
                vertical_damage_.bottom = tile_height + new_y - 1 + 4;
            } else {
                vertical_damage_.top = new_y - 4;
                vertical_damage_.bottom = old_y + 4;
            }
        }

        int current_pixel_x;
        int current_pixel_y;
        tile_to_pixel(field_1DDA4_, field_1DDA8_,
                      &current_pixel_x, &current_pixel_y);
        // Conversion may change the origin. Cache it only after that call,
        // then retain the same tile through all four altitude queries.
        const int altitude_x = field_1DDA4_;
        const int altitude_y = field_1DDA8_;
        int altitude = get_alt(altitude_x, altitude_y, 4);
        altitude += get_alt(altitude_x, altitude_y, 3);
        altitude += get_alt(altitude_x, altitude_y, 2);
        altitude += get_alt(altitude_x, altitude_y, 1);
        // SAR rounds negative sums down, unlike signed division by four.
        current_pixel_y += altitude >= 0 ? altitude / 4
                                        : -1 - ((-1 - altitude) / 4);

        int old_pixel_x;
        int old_pixel_y;
        tile_to_pixel(old_x, old_y, &old_pixel_x, &old_pixel_y);
        altitude = get_alt(old_x, old_y, 4);
        altitude += get_alt(old_x, old_y, 3);
        altitude += get_alt(old_x, old_y, 2);
        altitude += get_alt(old_x, old_y, 1);
        old_pixel_y += altitude >= 0 ? altitude / 4
                                    : -1 - ((-1 - altitude) / 4);

        const int dx = old_pixel_x - current_pixel_x;
        const int dy = old_pixel_y - current_pixel_y;
        canvas->copy(canvas, dx < 0 ? -dx : 0, dy < 0 ? -dy : 0,
                     dx > 0 ? dx : 0, dy > 0 ? dy : 0,
                     canvas->dib_.bmiHeader.biWidth,
                     -canvas->dib_.bmiHeader.biHeight);
        for (int corner = 0; corner < 4; ++corner) {
            MapCornerCopy[corner].x -= dx;
            MapCornerCopy[corner].y -= dy;
        }

        // Copy can change window state. Decide seam direction from the
        // current origin and extent, not the pre-copy tile geometry above.
        const int live_x = field_1DDA4_;
        const int tile_distance = live_x >= old_x ? live_x - old_x
                                                  : old_x - live_x;
        const bool crosses_seam =
            tile_distance > field_1DDD4_ + field_1DDCC_;
        const bool clear_right = crosses_seam ? live_x < old_x : dx < 0;
        const int exposed_width = dx < 0 ? -dx : dx;
        const int pixel_width = canvas->dib_.bmiHeader.biWidth;
        const int pixel_height = -canvas->dib_.bmiHeader.biHeight;
        horizontal_pixels.left = clear_right ? pixel_width - exposed_width : 0;
        horizontal_pixels.top = 0;
        horizontal_pixels.right = horizontal_pixels.left + exposed_width;
        horizontal_pixels.bottom = pixel_height;
        GraphicWin::fill(horizontal_pixels.left, 0, exposed_width,
                         pixel_height, 0);

        // Reload dimensions after the first fill, before the second one.
        const int live_height = -canvas->dib_.bmiHeader.biHeight;
        const int exposed_height = dy < 0 ? -dy : dy;
        vertical_pixels.left = 0;
        vertical_pixels.top = dy < 0 ? live_height - exposed_height : 0;
        vertical_pixels.right = canvas->dib_.bmiHeader.biWidth;
        vertical_pixels.bottom = vertical_pixels.top + exposed_height;
        GraphicWin::fill(0, vertical_pixels.top, vertical_pixels.right,
                         exposed_height, 0);
    }

    // Clearing may have disabled incremental rendering. Full rendering
    // inherits the current clip and skips the incremental feedback/reset.
    if (field_21A44_ == 0) {
        const int faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                                ? -1 : ConsoleGlobal.faction_id_;
        gen_map(faction, draw_type < 0);
    } else {
        if (IsRectEmpty(&vertical_pixels)) {
            canvas->set_clip(&horizontal_pixels);
            const int faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                                    ? -1 : ConsoleGlobal.faction_id_;
            gen_map(faction, draw_type < 0);
        } else if (IsRectEmpty(&horizontal_pixels)) {
            vertical_pixels.top -= 10;
            canvas->set_clip(&vertical_pixels);
            const int faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                                    ? -1 : ConsoleGlobal.faction_id_;
            gen_map(faction, draw_type < 0);
        } else {
            vertical_pixels.top -= 10;
            const MapTileDamageRect saved_vertical = vertical_damage_;
            vertical_damage_.left = horizontal_damage_.right;
            canvas->set_clip(&horizontal_pixels);
            int faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                              ? -1 : ConsoleGlobal.faction_id_;
            gen_map(faction, draw_type < 0);
            vertical_damage_ = saved_vertical;

            // Save this rectangle AFTER the first generation: that call
            // may have changed it. The old seed omitted this second save.
            const MapTileDamageRect saved_horizontal = horizontal_damage_;
            horizontal_damage_.left = horizontal_damage_.right;
            canvas->set_clip(&vertical_pixels);
            faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0
                          ? -1 : ConsoleGlobal.faction_id_;
            gen_map(faction, draw_type < 0);
            horizontal_damage_ = saved_horizontal;
        }
        if (field_1DD80_ != 0) {
            int mouse_x;
            int mouse_y;
            ConsoleGlobal.get_mouse_pos(&mouse_x, &mouse_y);
            on_mouse_move(mouse_x, mouse_y);
        }
        canvas->set_clip(&canvas->rect2_);
    }
    if (draw_type != 0) {
        on_update();
    }
}

namespace {
int mapwin_dest_add(int left, int right);
int mapwin_dest_sub(int left, int right);
int mapwin_dest_mul(int left, int right);

int map_dest_signed(uint32_t value) {
    return value <= 0x7fffffffU ? static_cast<int>(value)
                              : -1 - static_cast<int>(0xffffffffU - value);
}

int mapwin_dest_add(int left, int right) {
    return map_dest_signed(static_cast<uint32_t>(left) + static_cast<uint32_t>(right));
}

int mapwin_dest_sub(int left, int right) {
    return map_dest_signed(static_cast<uint32_t>(left) - static_cast<uint32_t>(right));
}

int mapwin_dest_mul(int left, int right) {
    return map_dest_signed(static_cast<uint32_t>(left) * static_cast<uint32_t>(right));
}

int map_dest_abs(int value) {
    return value < 0 ? mapwin_dest_sub(0, value) : value;
}

int map_dest_half(int value) {
    return value >= 0 ? value / 2 : -1 - ((-1 - value) / 2);
}

int map_dest_distance(int x1, int y1, int x2, int y2) {
    int dx = map_dest_abs(mapwin_dest_sub(x1, x2));
    if ((MapIsFlat & 1) == 0 && dx > map_dest_signed(MapLongitude)) {
        dx = mapwin_dest_sub(MapLongitudeBounds, dx);
    }
    return mapwin_dest_add(dx, map_dest_abs(mapwin_dest_sub(y1, y2)));
}

bool map_dest_in_view(const MapWin &window, int x, int y) {
    const int top = window.field_1DDA8_;
    if (y < top) {
        return false;
    }
    const int bottom = mapwin_dest_add(mapwin_dest_add(window.field_1DDD8_,
                                                window.field_1DDD0_), top);
    if (y >= bottom) {
        return false;
    }
    const int left = window.field_1DDA4_;
    const int right = mapwin_dest_add(mapwin_dest_add(window.field_1DDD4_,
                                               window.field_1DDCC_), left);
    // Normalize a probe, not the endpoints passed to direct_dest_line.
    if ((MapIsFlat & 1) == 0) {
        if (x < left) {
            x = mapwin_dest_add(x, MapLongitudeBounds);
        }
        if (x >= right) {
            x = mapwin_dest_sub(x, MapLongitudeBounds);
        }
    }
    return x >= left && x < right;
}
}

// ORIGINAL: 0x004672C0 ?direct_dest_line@MapWin@@QAEXHHHHHHHH@Z 0x004672C0-0x0046742C
// RECOVERY_BINDING: v1 {"evidence": ["Original4672C0..46742C; retired seed git:b55fcf8b:src/recovered/units/004672c0.cpp records this identity and exact span.", "Original467388 uses source Y as the default color; both tile conversions precede center-offset reads, and cover rendering performs both screen-to-client conversions."], "reason": "MapWin projects both endpoints and draws the route edge on either CoverWindow or its own Buffer. Preserve both target branches and the original phase/color selection; the patterned Buffer dependency remains explicitly unfinished."}
void MapWin::direct_dest_line(int x1, int y1, int x2, int y2, int triad,
                             int pattern_on, int pattern_off, int cover) {
    Win::update_cursor(0, 1);
    int pixel_x1;
    int pixel_y1;
    int pixel_x2;
    int pixel_y2;
    tile_to_pixel(x1, y1, &pixel_x1, &pixel_y1);
    tile_to_pixel(x2, y2, &pixel_x2, &pixel_y2);
    // Both center offsets are read after BOTH conversions, then reused.
    const int center_x = map_dest_signed(field_1DDBC_);
    const int center_y = map_dest_signed(field_1DDC0_);
    pixel_x1 = mapwin_dest_add(pixel_x1, center_x);
    pixel_y1 = mapwin_dest_add(pixel_y1, center_y);
    pixel_x2 = mapwin_dest_add(pixel_x2, center_x);
    pixel_y2 = mapwin_dest_add(pixel_y2, center_y);

    int color;
    switch (triad) {
    case 0:
        color = 0xfa;
        break;
    case 1:
        color = 0xfe;
        if (cover == 0) {
            pixel_x1 = mapwin_dest_add(pixel_x1, 1);
            pixel_x2 = mapwin_dest_add(pixel_x2, 1);
            pixel_y1 = mapwin_dest_add(pixel_y1, 1);
            pixel_y2 = mapwin_dest_add(pixel_y2, 1);
        }
        break;
    case 2:
        color = 0xf9;
        if (cover == 0) {
            pixel_y1 = mapwin_dest_sub(pixel_y1, 1);
            pixel_y2 = mapwin_dest_sub(pixel_y2, 1);
        }
        break;
    default:
        // Original467388 loads the original source Y argument, not triad.
        color = y1;
        break;
    }
    if (field_1DD84_ != 0) {
        color = 0xfb;
    }

    if (cover != 0) {
        Win::screen_to_client(&pixel_x1, &pixel_y1);
        Win::screen_to_client(&pixel_x2, &pixel_y2);
        CoverWindow.Buffer::line(pixel_x1, pixel_y1, pixel_x2, pixel_y2, color);
    } else {
        const int phase = map_dest_signed(
            static_cast<uint32_t>(mapwin_dest_sub(mapwin_dest_add(pattern_on, pattern_off), 1)) &
            static_cast<uint32_t>(PlanLineCounter));
        // This genuine nine-argument dependency is not omitted: its owning
        // Buffer implementation is a separate, explicit recovery obligation.
        Buffer::line(pixel_x1, pixel_y1, pixel_x2, pixel_y2,
                     0, color, pattern_on, pattern_off, phase);
    }
}

// ORIGINAL: 0x00467430 ?dest_line@MapWin@@QAEXHHHHHHHHHHH@Z 0x00467430-0x00467958
// RECOVERY_BINDING: v1 {"evidence": ["Original467430..467958; retired seed git:b55fcf8b:src/recovered/units/00467430.cpp records this identity and exact span.", "Original467481/467483 establish connected-region admission;4676DD tests the pass index, not the per-pass step count. Cost writes46759x/467718/46792x own the separate7D3B70 scalar."], "reason": "MapWin owns route-preview traversal and cost publication. Original pass0 validates the whole route; pass1 charges and draws each edge. Connected land/sea uses path traversal and air/disconnected routes use the direct fallback."}
void MapWin::dest_line(int x1, int y1, int x2, int y2, int proto_id,
                      int faction, int triad, int repeat,
                      int pattern_on, int pattern_off, int cover) {
    bool fallback = false;
    if (triad == 0) {
        // Original467481/483: equal regions RETAIN flag0 (path mode).
        fallback = map_loc(x1, y1)->region != map_loc(x2, y2)->region;
    } else if (triad == 1) {
        const int base1 = base_at(x1, y1);
        const int base2 = base_at(x2, y2);
        // The map pointer/stride are loaded after the base_at callbacks.
        if (base1 >= 0) {
            if (base2 >= 0) {
                fallback = port_to_port(base1, base2) == 0;
            } else {
                fallback = base_on_sea(base1, map_loc(x2, y2)->region) == 0;
            }
        } else if (base2 >= 0) {
            fallback = base_on_sea(base2, map_loc(x1, y1)->region) == 0;
        } else {
            fallback = map_loc(x1, y1)->region != map_loc(x2, y2)->region;
        }
    } else if (triad == 2) {
        fallback = true;
    }

    // Pass0 validates the WHOLE route. Pass1 charges and draws EVERY edge,
    // including its first edge. The seed confused this pass index with the
    // distinct per-pass step counter at original ebp-18.
    for (int pass = 0; pass < 2 && !fallback; ++pass) {
        Win::update_cursor(0, 1);
        int current_x = x1;
        int current_y = y1;
        int steps = 0;
        int last_direction = -1;
        MapDestinationCost = 0;

        for (;;) {
            Win::update_cursor(0, 1);
            int flags = repeat != 0 ? 0x80 : 0xc0;
            if (Weapon[VehPrototypes[proto_id].weapon_id].offense_rating == 0) {
                const int project_base = GameDataStateStorage.fields_.secret_project_.xenoempathy_dome();
                if (project_base < 0 || Bases[project_base].faction_id_current != faction) {
                    flags |= 0x10;
                }
            }

            int direction;
            const int distance = map_dest_signed(
                static_cast<uint32_t>(map_dest_distance(current_x, current_y, x2, y2)) &
                0xfffffffeU);
            if (distance > 2) {
                direction = Paths.find(current_x, current_y, x2, y2,
                                       proto_id, faction, flags, last_direction);
                if (direction < 0) {
                    fallback = true;
                    break;
                }
            } else {
                direction = compass_move(current_x, current_y, x2, y2);
            }
            last_direction = direction;
            int next_x = mapwin_dest_add(current_x, VehMoveDeltaX[direction]);
            if ((MapIsFlat & 1) == 0) {
                if (next_x < 0) {
                    next_x = mapwin_dest_add(next_x, MapLongitudeBounds);
                } else if (next_x >= MapLongitudeBounds) {
                    next_x = mapwin_dest_sub(next_x, MapLongitudeBounds);
                }
            }
            const int next_y = mapwin_dest_add(current_y, VehMoveDeltaY[direction]);

            if (pass != 0) {
                if (proto_id >= 0) {
                    const int speed = speed_proto(proto_id);
                    const int cost = hex_cost(proto_id, faction,
                                              current_x, current_y, next_x, next_y,
                                              speed == map_dest_signed(Rules.move_rate_roads));
                    // hex_cost can update shared state: load the cost AFTER it.
                    MapDestinationCost = mapwin_dest_add(MapDestinationCost, cost);
                }
                if (map_dest_in_view(*this, current_x, current_y) ||
                    map_dest_in_view(*this, next_x, next_y)) {
                    direct_dest_line(current_x, current_y, next_x, next_y,
                                     triad, pattern_on, pattern_off, cover);
                }
            }

            // Arrival bypasses fog rejection and the safety step increment.
            if (next_x == x2 && next_y == y2) {
                break;
            }
            if ((PlayersData[faction].flags & 0x200) == 0 &&
                (map_loc(next_x, next_y)->visibility &
                 static_cast<uint8_t>(1U << (static_cast<uint32_t>(faction) & 31))) == 0 &&
                (GameDataStateStorage.fields_.game_state_ & 0x80) == 0 && (GameDataStateStorage.fields_.game_rules_ & 0x100) == 0) {
                fallback = true;
            }
            steps = mapwin_dest_add(steps, 1);
            if (steps > mapwin_dest_mul(MapLongitudeBounds, 2)) {
                fallback = true;
            }
            if (fallback) {
                break;
            }
            current_x = next_x;
            current_y = next_y;
        }
    }

    if (fallback) {
        MapDestinationCost = mapwin_dest_mul(
            map_dest_half(map_dest_distance(x1, y1, x2, y2)),
            map_dest_signed(Rules.move_rate_roads));
        direct_dest_line(x1, y1, x2, y2, triad, pattern_on, pattern_off, cover);
    }
}

namespace {
int mouse_feedback_signed(uint32_t value) {
    return value <= 0x7fffffffU ? static_cast<int>(value)
                              : -1 - static_cast<int>(0xffffffffU - value);
}

int mouse_feedback_add(int left, int right) {
    return mouse_feedback_signed(static_cast<uint32_t>(left) +
                                 static_cast<uint32_t>(right));
}

int mouse_feedback_sub(int left, int right) {
    return mouse_feedback_signed(static_cast<uint32_t>(left) -
                                 static_cast<uint32_t>(right));
}

int mouse_feedback_mul(int left, int right) {
    return mouse_feedback_signed(static_cast<uint32_t>(left) *
                                 static_cast<uint32_t>(right));
}

void mouse_feedback_store_x(MapWin &window, int index, int value) {
    // The UI cycles count0..3 even in repeat mode (517E0A..517E66).
    // Thus a five-segment path genuinely writes the next physical field:
    // x[5] at1DDF8 aliases y[0]. Do not widen the five-element array.
    if (index == 5) {
        window.feedback_y_[0] = value;
    } else {
        window.feedback_x_[index] = value;
    }
}

void mouse_feedback_store_y(MapWin &window, int index, int value) {
    // Original y[5] at1DE0C aliases the cached hover-X field.
    if (index == 5) {
        window.field_1DE0C_ = static_cast<uint32_t>(value);
    } else {
        window.feedback_y_[index] = value;
    }
}
}

// The custom title strip returns HTCLIENT; points outside it use Win's normal
// non-client hit test through the genuine virtual GraphicWin/Win base.
// ORIGINAL: 0x0046F660 ?on_nc_hittest@MapWin@@QAEXHH@Z 0x0046F660-0x0046F6F9
// RECOVERY_BINDING: v1 {"evidence": ["git:b55fcf8b:src/unrecovered/0046f660.cpp; retained identity, exact span, and return-register evidence.", "src/mapwin.h:MapWin::on_nc_hittest and src/win.h:Win::on_nc_hittest/client_rect_ establish the owner, inherited hit-test contract, and named client rectangle fields.", "src/adjustor_thunks.cpp:adjust_map_win1_on_nc_hittest and adjust_map_win5_on_nc_hittest call the MapWin implementation through genuine virtual-base adjustment."], "reason": "Home the original map-window non-client hit test in its typed owner, returning the client hit code for the custom title strip and delegating other points to the genuine Win base."}
// symbol    ?on_nc_hittest@MapWin@@UAEHHH@Z
// TRIED (retired seed git:b55fcf8b:src/unrecovered/0046f660.cpp): the seed's int return was contradicted by its standalone scaffold's void declaration (C2556/C2371); keeping void dropped the mov eax,1 custom-hit result. Its remaining MISMATCH #6 ('mov' vs 'push') concerns instruction scheduling around the in_box/RECT stores.
int MapWin::on_nc_hittest(int x, int y) {
    RECT title_strip;
    title_strip.left = client_rect_.right - client_rect_.left - 0x5b;
    title_strip.top = 0xe;
    title_strip.right = title_strip.left + 0x50;
    title_strip.bottom = 0x1f;
    if (in_box(x, y, &title_strip)) {
        return 1;
    }
    return Win::on_nc_hittest(x, y);
}

// ORIGINAL: 0x0046F000 ?on_mouse_move@MapWin@@QAEXHH@Z 0x0046F000-0x0046F651
// RECOVERY_BINDING: v1 {"evidence": ["Original46F000..46F651; retired seed git:b55fcf8b:src/unrecovered/0046f000.cpp records this identity and exact span.", "Original517DD3..517E66 permits repeat mode with waypoint count3; endpoint x[5]/y[5] physically alias y[0]/hoverX. The C++ explicitly names those fields rather than overrunning arrays.", "Original46F0E1/46F118 skip directly to CoverWindow update;46F3A2..46F3B0 retains the base-path endpoint quirk;46F5DD tests the shared string address."], "reason": "The original MapWin/PlanWin/Console Win-branch slot17 entries4218B0/48BFA0/51DE70 dispatch this two-coordinate notification. Genuine virtual inheritance owns the receiver adjustment; it is distinct from the four-argument Win mouse router."}
// TRIED (retired seed git:b55fcf8b:src/unrecovered/0046f000.cpp): 'this' treated as a secondary-base receiver (MapWin front recovered via self - 0x21a6c, verified against field_1EF4C_/field_1EF50_ in the existing MapWin scaffold layout), with the per-unit waypoint loop and vbtable-adjusted set_cursor thunk transcribed mechanically field-offset-by-field-offset against raw disassembly. It compiles and reaches instruction #3 before diverging (prologue register-save order); not chased further given the function's size and fixed-point waypoint bookkeeping in the middle third.
// symbol    ?on_mouse_move@MapWin@@UAEXHH@Z
void MapWin::on_mouse_move(int pixel_x, int pixel_y) {
    if (MapMouseFeedbackBusy != 0 || UnkGlobal0093A934 != 0 ||
        ConsoleControlTurnActive != 0 || MapInputEnabled == 0 ||
        field_1DD78_ != 0) {
        return;
    }

    int tile_x;
    int tile_y;
    if (pixel_to_tile(pixel_x, pixel_y, &tile_x, &tile_y) != 0 ||
        tile_y < 0 || tile_y >= MapLatitudeBounds ||
        tile_x < 0 || tile_x >= MapLongitudeBounds) {
        return;
    }
    if (field_1DD8C_ != 0) {
        editor(tile_x, tile_y, field_1DD8C_ == 2);
        return;
    }

    if (ConsoleGlobal.selecting(tile_x, tile_y) == 0) {
        if (cursor_sprite_ != 0 || cursor_resource_id_ != 0x7f00) {
            Win::set_cursor(0x7f00);
            cursor_resource_id_ = 0x7f00;
            cursor_sprite_ = 0;
        }
        return;
    }

    // 46F0E1 and46F118 go directly to46F601, not the text builder.
    if (field_1DD80_ == 0) {
        CoverWindow.soft_update();
        return;
    }
    MapMouseFeedbackBusy = 1;
    do_all_non_input();
    MapMouseFeedbackBusy = 0;
    if (field_1DE0C_ == static_cast<uint32_t>(tile_x) &&
        feedback_hover_y_ == tile_y) {
        CoverWindow.soft_update();
        return;
    }

    CoverWindow.fill(9);
    field_1DE0C_ = static_cast<uint32_t>(tile_x);
    feedback_hover_y_ = tile_y;

    const int vehicle = ConsoleGlobal.selected_vehicle_;
    if (vehicle < 0) {
        Win::update_cursor(0, 1);
        feedback_proto_ = 10;
        feedback_x_[0] = Bases[field_1DE24_].x;
        feedback_y_[0] = Bases[field_1DE24_].y;
        // Original46F3A2 loads tile_y;46F3B0 writes the NEXT X slot.
        // No store initializes y[1] on this branch. Preserve this quirk.
        feedback_x_[1] = tile_y;
        feedback_triad_ = 2;
        feedback_repeat_ = 0;
        feedback_segment_count_ = 1;
        dest_line(feedback_x_[0], feedback_y_[0],
                  feedback_x_[1], feedback_y_[1],
                  feedback_proto_, ConsoleGlobal.faction_id_,
                  feedback_triad_, feedback_repeat_, 3, 5, 1);
    } else {
        feedback_proto_ = Vehs[vehicle].proto_id;
        feedback_repeat_ = ConsoleGlobal.field_23D10_ == 8;
        if (feedback_repeat_ != 0 ||
            (ConsoleGlobal.field_23D10_ == 1 &&
             (Vehs[vehicle].state & 0x02000000U) != 0)) {
            Vehs[vehicle].waypoint_x[Vehs[vehicle].waypoint_count] =
                static_cast<int16_t>(tile_x);
            Vehs[vehicle].waypoint_y[Vehs[vehicle].waypoint_count] =
                static_cast<int16_t>(tile_y);
        } else if (ConsoleGlobal.field_23D10_ == 1) {
            Vehs[vehicle].waypoint_x[0] = static_cast<int16_t>(tile_x);
            Vehs[vehicle].waypoint_y[0] = static_cast<int16_t>(tile_y);
            Vehs[vehicle].waypoint_count = 0;
        }
        feedback_triad_ = Chassis[VehPrototypes[Vehs[vehicle].proto_id].chassis_id].triad;

        // The original reloads both count and repeat each iteration, then
        // pumps before reading the next segment's coordinates/count again.
        for (int segment = 0;
             segment < Vehs[vehicle].waypoint_count + 1 +
                           (feedback_repeat_ != 0 && Vehs[vehicle].waypoint_count != 0);
             ++segment) {
            Win::update_cursor(0, 1);
            const int start_x = segment == 0 ? Vehs[vehicle].x
                                            : Vehs[vehicle].waypoint_x[segment - 1];
            const int start_y = segment == 0 ? Vehs[vehicle].y
                                            : Vehs[vehicle].waypoint_y[segment - 1];
            const int next = (segment + 1) % (Vehs[vehicle].waypoint_count + 2);
            const int end_x = next == 0 ? Vehs[vehicle].x
                                       : Vehs[vehicle].waypoint_x[next - 1];
            const int end_y = next == 0 ? Vehs[vehicle].y
                                       : Vehs[vehicle].waypoint_y[next - 1];
            mouse_feedback_store_x(*this, segment, start_x);
            mouse_feedback_store_y(*this, segment, start_y);
            mouse_feedback_store_x(*this, segment + 1, end_x);
            mouse_feedback_store_y(*this, segment + 1, end_y);
            feedback_segment_count_ = segment + 1;
            // Arguments retain the locals even when the endpoint stores
            // alias earlier fields. They are not reloaded from the arrays.
            dest_line(start_x, start_y, end_x, end_y,
                      feedback_proto_, ConsoleGlobal.faction_id_,
                      feedback_triad_, feedback_repeat_, 3, 5, 1);
        }
    }

    StringTemp[0] = '\0';
    if (ConsoleGlobal.field_23D10_ == 8) {
        strcat(StringTemp, label_get(410));
    } else if (field_1DD84_ != 0) {
        strcat(StringTemp, label_get(433));
    } else {
        strcat(StringTemp, label_get(411));
    }
    strcat(StringTemp, " ");
    // These are the original conversion's saved outputs, not a second call.
    say_loc(StringTemp, tile_x, tile_y, 0, 2, 0);
    strcat(StringTemp, ": ");
    strcat(StringTemp, label_get(412));
    strcat(StringTemp, " = ");
    char number[0x40];
    _itoa(MapDestinationCost / mouse_feedback_signed(Rules.move_rate_roads), number, 10);
    strcat(StringTemp, number);
    if (MapDestinationCost % mouse_feedback_signed(Rules.move_rate_roads) != 0) {
        strcat(StringTemp, " ");
        _itoa(MapDestinationCost % mouse_feedback_signed(Rules.move_rate_roads), number, 10);
        strcat(StringTemp, number);
        strcat(StringTemp, "/");
        _itoa(mouse_feedback_signed(Rules.move_rate_roads), number, 10);
        strcat(StringTemp, number);
    }

    CoverWindow.set_text_color(0x9f, 0x41, 1, 1);
    CoverWindow.set_font(find_font(12, 1), 0, 0, 0);
    const int width = ConsoleGlobal.dib_.bmiHeader.biWidth;
    const int text_width = CoverWindow.text_width(StringTemp);
    const int text_x = mouse_feedback_sub(pixel_x,
        mouse_feedback_mul(text_width, pixel_x) / width);
    const int text_y = mouse_feedback_add(
        mouse_feedback_signed(ConsoleGlobal.field_1DDB8_), pixel_y);
    // 46F5DD tests StringTemp's ADDRESS, not its first character. It is
    // always nonnull for owned storage; empty strings still reach write_l.
    CoverWindow.write_l(StringTemp, text_x, text_y, static_cast<int>(strlen(StringTemp)));
    CoverWindow.soft_update();
}

namespace {
int map_radius_wrap_x(int x) {
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) {
            x = mapwin_dest_add(x, MapLongitudeBounds);
        } else if (x >= MapLongitudeBounds) {
            x = mapwin_dest_sub(x, MapLongitudeBounds);
        }
    }
    return x;
}

bool map_radius_valid(int x, int y) {
    return y >= 0 && y < MapLatitudeBounds &&
           x >= 0 && x < MapLongitudeBounds;
}

int map_radius_half_width(int extent, int row) {
    // This is intentionally NOT a symmetric diamond. Original4693A4..3BB
    // narrows negative rows by their distance, but positive rows by parity.
    const int inset = row < 0 ? mapwin_dest_sub(0, row) : row & 1;
    return mapwin_dest_sub(extent, inset);
}
}

// Original4216F0..42180C: genuine MapWin thiscall, not the seed's stdcall.
// Proposed name; the original has no surviving decorated method identity.
// ORIGINAL: 0x004216F0 sub_4216f0 0x004216F0-0x0042180C
// RECOVERY_BINDING: v1 {"evidence": ["Original4216F0..42180C; retired seed git:b55fcf8b:src/unrecovered/004216f0.cpp records this identity and exact span; gen_radius469190 calls this method for overlay admission."], "reason": "The original uses a MapWin receiver and three stack arguments, testing its viewport and optional altitude margin. The seed stdcall declaration loses the genuine receiver; tile_in_view is a behavior-based provisional name."}
// TRIED (retired seed git:b55fcf8b:src/unrecovered/004216f0.cpp): the synthetic Obj4216F0 __thiscall receiver was rejected because disassembly reads [ecx+0x1ddaN] without loading ecx from the stack; this is the this-not-a-parameter signal, so IDA's guessed __stdcall(int,int,int) was wrong (a proposal, not a catalogue fact; no mangled name survives). The seed landed at 0.74 similarity: its tile-pointer expression collapsed arithmetic that the original keeps as an explicit zeroed accumulator beside imul, a register-allocation difference that plain restatement did not move.
// symbol    ?tile_in_view@MapWin@@QAEHHHH@Z
int MapWin::tile_in_view(int x, int y, int margin) {
    if (margin != 0 && map_radius_valid(x, y)) {
        const int climate = map_loc(x, y)->climate;
        if ((climate & 0xe0) > 0x60 &&
            mapwin_dest_add(mapwin_dest_sub(y, climate >> 5), 3) <
                mapwin_dest_add(field_1DDA8_, margin)) {
            return 0;
        }
    }
    const int top = field_1DDA8_;
    if (y < mapwin_dest_add(top, margin)) {
        return 0;
    }
    const int bottom = mapwin_dest_add(
        mapwin_dest_sub(mapwin_dest_add(field_1DDD8_, field_1DDD0_), margin), top);
    if (y >= bottom) {
        return 0;
    }
    const int left = mapwin_dest_add(field_1DDA4_, margin);
    const int right = mapwin_dest_add(
        mapwin_dest_sub(mapwin_dest_add(field_1DDD4_, field_1DDCC_), margin), field_1DDA4_);
    if ((MapIsFlat & 1) == 0) {
        if (x < left) {
            x = mapwin_dest_add(x, MapLongitudeBounds);
        }
        if (x >= right) {
            x = mapwin_dest_sub(x, MapLongitudeBounds);
        }
    }
    return x >= left && x < right;
}

// Original4712D0..471316, historically claimed exact as mapwin_unk1.
// The adapter itself is genuine code, with a nullable derived-to-base cast;
// its gen_terrain_poly dependency remains the retained renderer obligation.
// ORIGINAL: 0x004712D0 mapwin_unk1 0x004712D0-0x00471316 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Original4712D0..471316; retired seed git:b55fcf8b:src/recovered/units/004712d0.cpp records this identity and exact span, including its BYTE_EXACT obligation and historical ABI notes.", "Original conditional receiver conversion reads the GraphicWin virtual-base displacement and adds the Buffer subobject offset. C++ base conversion supplies the same null handling without manual vbtable arithmetic."], "reason": "This nine-argument cdecl adapter performs a nullable genuine MapWin-to-Buffer conversion before entering the common ten-argument renderer. Preserve its original mapwin_unk1 identity and historical seed obligation; gen_window_terrain describes the recovered source role."}
// symbol    ?gen_window_terrain@@YAHPAVMapWin@@HHHHHHHH@Z
int __cdecl gen_window_terrain(MapWin *window, int zoom, int faction,
                              int base_id, int flags, int x, int y,
                              int pixel_x, int pixel_y) {
    return gen_terrain_poly(static_cast<Buffer *>(window), window,
                            zoom, faction, base_id, flags,
                            x, y, pixel_x, pixel_y);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00466CA0 ?draw_vehicles@MapWin@@QAEXHHHHH@Z 0x00466CA0-0x0046718F FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00466ca0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void MapWin::draw_vehicles(int tile_x, int tile_y, int pixel_x, int pixel_y,
                           int faction_id) {
    int draw_state = 2;
    const uint32_t window_flags = field_1DD70_;
    if (MapBaseDisplayOverride == 0 && ControlTurnPhase == 0 &&
        (GameDataStateStorage.fields_.game_rules_ & 0x80) == 0) {
        return;
    }
    if ((window_flags & 0x01000000U) != 0 ||
        (window_flags & 0x802000C0U) == 0 ||
        (window_flags & 0x00001000U) != 0) {
        return;
    }

    const int half_x = tile_x >> 1;
    bool draw_base = false;
    if ((window_flags & 0x80000000U) == 0 ||
        (window_flags & 0x80U) != 0) {
        if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0 ||
            (PlayersData[faction_id].flags & 0x200U) != 0 ||
            (map_loc(tile_x, tile_y)->visibility & (1U << faction_id)) != 0) {
            draw_base = true;
        }
    }
    if (draw_base) {
        draw_bases(tile_x, tile_y, pixel_x, pixel_y, faction_id);
    }
    if ((window_flags & 0x80000000U) != 0) return;

    int target = -1;
    if (VehDrawSuppressedVehicle != -1 &&
        Vehs[VehDrawSuppressedVehicle].x == tile_x &&
        Vehs[VehDrawSuppressedVehicle].y == tile_y) {
        target = VehDrawSuppressedVehicle;
        draw_state = 3;
        goto draw_target;
    }
    if (VehDrawSelection689F28 >= 0 &&
        Vehs[VehDrawSelection689F28].x == tile_x &&
        Vehs[VehDrawSelection689F28].y == tile_y) {
        target = VehDrawSelection689F28;
        if (VehPrototypes[Vehs[target].proto_id].chassis_id == 8) {
            target = Vehs[target].next_veh_id_stack;
            if (target < 0) return;
        }
        goto draw_target;
    }
    if (VehDrawSelection689F2C >= 0 &&
        Vehs[VehDrawSelection689F2C].x == tile_x &&
        Vehs[VehDrawSelection689F2C].y == tile_y) {
        target = VehDrawSelection689F2C;
        goto draw_target;
    }

    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0 &&
        (PlayersData[faction_id].flags & 0x200U) == 0 &&
        (map_loc(tile_x, tile_y)->visibility & (1U << faction_id)) == 0) {
        return;
    }
    draw_state = 3;
    if (Vehs[ConsoleGlobal.selected_vehicle_].x == tile_x &&
        Vehs[ConsoleGlobal.selected_vehicle_].y == tile_y &&
        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << Vehs[ConsoleGlobal.selected_vehicle_].faction_id)) != 0 &&
        veh_selectable(ConsoleGlobal.selected_vehicle_) != 0 &&
        (Vehs[ConsoleGlobal.selected_vehicle_].state & 0x4200U) == 0) {
        target = ConsoleGlobal.selected_vehicle_;
        if (ConsoleGlobal.cursor_active_ != 0 &&
            ConsoleGlobal.field_23BF8_ != 0 &&
            ConsoleGlobal.field_23BE4_ != 0 &&
            Vehs[target].order == 0 && ConsoleGlobal.survey_mode_ == 0 &&
            (IsMultiplayerNet == 0 || ConsoleControlTurnActive == 0)) {
            if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x200U) != 0) return;
            int mouse_x;
            int mouse_y;
            static_cast<Win *>(this)->get_mouse_pos(&mouse_x, &mouse_y);
            int cursor_tile_x;
            int cursor_tile_y;
            pixel_to_tile(mouse_x, mouse_y, &cursor_tile_x, &cursor_tile_y);
            const int delta_x = abs(cursor_tile_x - Vehs[target].x);
            const int delta_y = abs(cursor_tile_y - Vehs[target].y);
            const int adjusted_x =
                (MapIsFlat & 1) == 0 && static_cast<int>(MapLongitude) < delta_x
                    ? static_cast<int>(MapLongitudeBounds) - delta_x : delta_x;
            if (((delta_y + adjusted_x) & ~1) < 3) return;
            draw_state = 0x403;
        }
    } else {
        target = ConsoleGlobal.selected_vehicle_;
        if ((window_flags & 0x00020040U) == 0) return;
        target = veh_at(tile_x, tile_y);
    }

    if ((window_flags & 0x00200000U) != 0) {
        target = veh_at(tile_x, tile_y);
        if (target < 0) return;
        while (Vehs[target].home_base_id !=
               static_cast<int16_t>(TutWinBaseWindow.focused_base_id())) {
            target = Vehs[target].next_veh_id_stack;
            if (target < 0) return;
        }
    }
    if (target < 0) return;

    if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0) {
        const Veh &vehicle = Vehs[target];
        if (vehicle.faction_id != ConsoleGlobal.faction_id_ ||
            (vehicle.flags & 0x440U) == 0x440U) {
            if ((vehicle.visibility & (1U << ConsoleGlobal.faction_id_)) == 0)
                return;
        }
    }

    {
        const Map &tile = *map_loc(tile_x, tile_y);
        if ((tile.bit & 1U) != 0 && (tile.val2 & 0x0FU) < 8) {
            if (ConsoleGlobal.cursor_active_ == 0 ||
                target != ConsoleGlobal.selected_vehicle_ ||
                veh_ready(target) == 0 || (Vehs[target].state & 0x4200U) != 0 ||
                ConsoleGlobal.field_23BE4_ == 0) {
                return;
            }
        }
    }

draw_target:
    veh_draw(static_cast<Buffer *>(this), target, pixel_x, pixel_y,
             field_1DD98_, draw_state, 1);
}

// Original468F90..469183: distinct from the five-argument tile renderer.
// ORIGINAL: 0x00468F90 ?draw_vehicles@MapWin@@QAEXHHHH@Z 0x00468F90-0x00469183
// RECOVERY_BINDING: v1 {"evidence": ["Original468F90..469183; retired seed git:b55fcf8b:src/recovered/units/00468f90.cpp records this identity and exact span; the original vehicle scan caps extension at10, unlike terrain and overlay scans."], "reason": "The four-argument MapWin overload scans a clipped radius and calls the distinct five-argument tile renderer. It preserves the capped upward extension, asymmetric rows and live viewport/center reads."}
void MapWin::draw_vehicles(int x, int y, int radius, int faction) {
    Buffer *const canvas = static_cast<Buffer *>(this);
    int span = mapwin_dest_add(mapwin_dest_mul(radius, 2), 2);
    int upper_tile_y = 32000;
    int ignored_x;
    pixel_to_tile(canvas->rect1_.left, canvas->rect1_.top,
                  &ignored_x, &upper_tile_y);
    while (upper_tile_y < mapwin_dest_sub(y, span) && span < 10) {
        span = mapwin_dest_add(span, 2);
    }
    const int extent = mapwin_dest_add(span, 2);
    if (extent < 0) {
        return;
    }
    for (int row = -extent; row <= extent + 1; ++row) {
        int half_width = map_radius_half_width(extent, row);
        if (radius == 0) {
            half_width = mapwin_dest_add(half_width, 2);
        }
        const int tile_y = mapwin_dest_add(y, row);
        for (int column = -half_width; column <= half_width; column += 2) {
            const int tile_x = map_radius_wrap_x(mapwin_dest_add(x, column));
            if (map_radius_valid(tile_x, tile_y) &&
                map_dest_in_view(*this, tile_x, tile_y)) {
                int pixel_x;
                int pixel_y;
                tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
                draw_vehicles(tile_x, tile_y, pixel_x,
                              mapwin_dest_sub(pixel_y, field_1DDC0_), faction);
            }
        }
    }
}

// Original469190..469718, not the register/goto seed's rectangle arguments.
// ORIGINAL: 0x00469190 ?gen_radius@MapWin@@QAEXHHHHH@Z 0x00469190-0x00469718
// RECOVERY_BINDING: v1 {"evidence": ["Original469190..469718; retired seed git:b55fcf8b:src/recovered/units/00469190.cpp records this identity and exact span.", "Original469333..469355 converts left/top then right/top, not the transposed seed arguments;4691FB fills via the genuine GraphicWin receiver. Radius-zero scan widening does not apply to the overlay scan."], "reason": "MapWin owns clipped terrain, vehicle and overlay generation. The original scan is asymmetric; top-left and top-right clip conversions determine terrain extension, while overlays later recompute from the left conversion."}
void MapWin::gen_radius(int x, int y, int radius, int faction, int skip_terrain) {
    RECT clip;
    if (compute_clip(x, y, radius, &clip) == 0) {
        return;
    }
    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->set_clip(&clip);
    if (radius == 0 && skip_terrain == 0) {
        if ((field_1DD70_ & 0x10000000U) == 0) {
            GraphicWin::fill(&clip, 0x41);
        }
        const int tile_y = mapwin_dest_sub(y, 3);
        for (int side = -1; side <= 1; side += 2) {
            const int tile_x = map_radius_wrap_x(mapwin_dest_add(x, side));
            if (map_radius_valid(tile_x, tile_y)) {
                int pixel_x;
                int pixel_y;
                tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
                gen_terrain_poly(canvas, this, field_1DD98_, faction, -1,
                                 field_1DD70_, tile_x, tile_y, pixel_x, pixel_y);
                on_map(tile_x, tile_y);
            }
        }
    }

    const int initial_span = mapwin_dest_add(mapwin_dest_mul(radius, 2), 2);
    int span = initial_span;
    int upper_left_y = 32000;
    int upper_right_y = 32000;
    int ignored_x;
    pixel_to_tile(clip.left, clip.top, &ignored_x, &upper_left_y);
    pixel_to_tile(clip.right, clip.top, &ignored_x, &upper_right_y);
    const int upper_y = upper_left_y < upper_right_y ? upper_left_y : upper_right_y;
    while (upper_y < mapwin_dest_sub(y, span)) {
        span = mapwin_dest_add(span, 2);
    }
    const int terrain_extent = mapwin_dest_add(span, 2);
    if (skip_terrain == 0) {
        for (int row = -terrain_extent; row <= terrain_extent + 1; ++row) {
            int half_width = map_radius_half_width(terrain_extent, row);
            if (radius == 0) {
                half_width = mapwin_dest_add(half_width, 2);
            }
            const int tile_y = mapwin_dest_add(y, row);
            for (int column = -half_width; column <= half_width; column += 2) {
                const int tile_x = map_radius_wrap_x(mapwin_dest_add(x, column));
                if (map_radius_valid(tile_x, tile_y) &&
                    map_dest_in_view(*this, tile_x, tile_y)) {
                    int pixel_x;
                    int pixel_y;
                    tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
                    gen_window_terrain(this, field_1DD98_, faction, -1,
                                       field_1DD70_, tile_x, tile_y, pixel_x, pixel_y);
                }
            }
        }
    }

    draw_vehicles(x, y, radius, faction);
    draw_base_names();
    if ((field_1DD70_ & 0x2000) != 0) {
        if (radius == 0) {
            const int tile_y = mapwin_dest_sub(y, 3);
            for (int side = -1; side <= 1; side += 2) {
                const int tile_x = map_radius_wrap_x(mapwin_dest_add(x, side));
                if (map_radius_valid(tile_x, tile_y)) {
                    gen_overlays(tile_x, tile_y);
                }
            }
        }
        span = initial_span;
        upper_left_y = 32000;
        pixel_to_tile(clip.left, clip.top, &ignored_x, &upper_left_y);
        while (upper_left_y < mapwin_dest_sub(y, span)) {
            span = mapwin_dest_add(span, 2);
        }
        const int overlay_extent = mapwin_dest_add(span, 2);
        for (int row = -overlay_extent; row <= overlay_extent + 1; ++row) {
            const int half_width = map_radius_half_width(overlay_extent, row);
            const int tile_y = mapwin_dest_add(y, row);
            for (int column = -half_width; column <= half_width; column += 2) {
                const int tile_x = map_radius_wrap_x(mapwin_dest_add(x, column));
                if (map_radius_valid(tile_x, tile_y) && tile_in_view(tile_x, tile_y, 0)) {
                    gen_overlays(tile_x, tile_y);
                }
            }
        }
    }
    if ((field_1DD70_ & 0x20000) != 0) {
        draw_legend();
    }
    canvas->set_clip(&canvas->rect2_);
}

// Original46A2A0..46A496. This consumes the SAME copied vertices produced
// by gen_terrain_poly, never a detached radius-local copy or address alias.
// ORIGINAL: 0x0046A2A0 ?draw_radius@MapWin@@QAEXHHHH@Z 0x0046A2A0-0x0046A496
// RECOVERY_BINDING: v1 {"evidence": ["Original46A2A0..46A496; retired seed git:b55fcf8b:src/recovered/units/0046a2a0.cpp records this identity and exact span.", "Original46A437 passes shared7D3BE8 vertices to the selected genuine radius Texture; signed zoom selects the texture. Console state is observed after generation and soft_update precedes optional do_all_draws."], "reason": "This unadjusted MapWin receiver owns radius generation, the live console cursor-overlay decision and the bounded update. It consumes the same four copied vertices as the terrain producer and scroll controller."}
void MapWin::draw_radius(int x, int y, int radius, int draw_type) {
    const int faction = (GameDataStateStorage.fields_.game_state_ & 0x80) != 0 ? -1 : ConsoleGlobal.faction_id_;
    gen_radius(x, y, radius, faction, draw_type < 0);

    if (ConsoleGlobal.cursor_active_ == 0) {
        const int network = IsMultiplayerNet;
        if ((network == 0 || ConsoleControlTurnActive == 0) && draw_type >= 0 &&
            (ConsoleGlobal.field_23BF8_ != 0 || ConsoleGlobal.survey_mode_ != 0) &&
            (ConsoleGlobal.field_23BE4_ != 0 || (network != 0 && (GameDataStateStorage.fields_.game_state_ & 2) != 0)) &&
            (field_1DD78_ == 0 || (field_1DD70_ & 0x80000000U) != 0)) {
            const int history = ConsoleGlobal.cursor_history_index_;
            const int cursor_y = ConsoleGlobal.cursor_history_y_[history];
            const int cursor_x = ConsoleGlobal.cursor_history_x_[history];
            if (map_dest_in_view(*this, cursor_x, cursor_y)) {
                Texture &texture = map_dest_signed(field_1DD98_) <= -8
                                       ? g_RADIUS2_TEXTURE : g_RADIUS1_TEXTURE;
                texture.draw_trans(static_cast<Buffer *>(this), MapCornerCopy, 0);
            }
        }
    }
    if (draw_type != 0) {
        RECT clip;
        if (compute_clip(x, y, radius, &clip) != 0) {
            GraphicWin::soft_update(&clip);
            if (draw_type >= 2) {
                do_all_draws();
            }
        }
    }
}

// The three-argument overload is the original radius-1 convenience call.
// ORIGINAL: 0x0046A530 ?draw_radius@MapWin@@QAEXHHH@Z 0x0046A530-0x0046A54A BYTE_EXACT
void MapWin::draw_radius(int x, int y, int draw_type) {
    draw_radius(x, y, 1, draw_type);
}

/*
Purpose: Repaint every live MapWin slot - the FREE draw_map, not the method
         above. Mirrors draw_tile/draw_tiles in mapwin.h: slot 0 is exempt
         from the activity gate, every other slot only draws when its
         MapWinActiveOffset dword is non-zero.
// ORIGINAL: 0x0046B190 ?draw_map@@YAXH@Z 0x0046B190-0x0046B1CA BYTE_EXACT
// size      58 bytes
// prototype void (__cdecl ?draw_map@@YAXH@Z)(int)
// callers   68   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A550
Return Value: n/a
Status: Complete
*/
void __cdecl draw_map(int draw_type) {
    // A do-while walking a raw cursor over the table, not draw_tile's
    // indexed for-loop: the image parks the table's BASE ADDRESS in a
    // register (`mov esi, 0x7d3c3c`) and compares the cursor against it
    // directly (`cmp esi, 0x7d3c3c`) to recognise slot 0, rather than
    // comparing an index against 0. `int`, not a pointer: the loop
    // termination check is `jl`, a SIGNED compare, which a real pointer
    // comparison would not give.
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
        if (window != nullptr) {
            if (cursor == reinterpret_cast<int>(MapWinTable) ||
                *reinterpret_cast<const volatile uint32_t *>(
                    reinterpret_cast<const uint8_t *>(window) +
                    MapWinActiveOffset) != 0) {
                // THE METHOD, not the seam directly: `window->draw_map(...)`
                // compiles the image's `call rel32` to
                // `?draw_map@MapWin@@QAEXH@Z`, where dispatching through
                // MapWinOriginalDrawMap here would be an indirect call
                // through the seam's own storage.
                window->draw_map(draw_type);
            }
        }
        cursor += 4;
    } while (cursor < end);
}

/*
Purpose: Repaint the radius around a tile on every live MapWin slot. Like
         the free draw_map above, call the genuine MapWin method directly;
         the original call site is a direct rel32 to 0x46A2A0.
// ORIGINAL: 0x0046AEF0 ?draw_radius@@YAXHHHH@Z 0x0046AEF0-0x0046AF38 BYTE_EXACT
// size      72 bytes
// prototype void (__cdecl ?draw_radius@@YAXHHHH@Z)(int, int, int, int)
// callers   2   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x0046A2A0
Return Value: n/a
Status: Complete
*/
void __cdecl draw_radius(int x_coord, int y_coord, int a3, int draw_type) {
    int cursor = reinterpret_cast<int>(MapWinTable);
    const int end = cursor + static_cast<int>(MapWinTableSlots) * 4;
    do {
        MapWin *const window = *reinterpret_cast<MapWin *const *>(cursor);
        if (window != nullptr) {
            if (cursor == reinterpret_cast<int>(MapWinTable) ||
                *reinterpret_cast<const volatile uint32_t *>(
                    reinterpret_cast<const uint8_t *>(window) +
                    MapWinActiveOffset) != 0) {
                window->draw_radius(x_coord, y_coord, a3, draw_type);
            }
        }
        cursor += 4;
    } while (cursor < end);
}

// The first extra receiver is WorldClimateWorldWin's complete embedded MapWin:
// original WorldWin storage 0x008E9F60 + constructor/destructor member offset
// 0x152C = 0x008EB48C. Its lifetime belongs to WorldWin, not another global.
// Only the second extra receiver still lacks a source-owned identity.
static MapWin *const MapWinTerrainFixupExtra2 =
    reinterpret_cast<MapWin *>(0x006C5C9C);

/*
Purpose: Fix up terrain on every live MapWin slot, then the embedded world
         map, one unresolved window and the shared farm display, in that order.
// ORIGINAL: 0x00471240 ?mapwin_terrain_fixup@@YAXXZ 0x00471240-0x00471293 BYTE_EXACT
// size      83 bytes
// prototype void (__cdecl ?mapwin_terrain_fixup@@YAXXZ)()
// callers   1   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x004711A0
Return Value: n/a
Status: Complete
*/
void __cdecl mapwin_terrain_fixup() {
    for (unsigned int slot = 0; slot < MapWinTableSlots; ++slot) {
        // BUG IN THE ORIGINAL: unlike draw_tile/draw_tiles/draw_radius, this
        // reads the active field at +0x1DD74 for every non-zero slot
        // WITHOUT first checking that the slot itself is non-null - only
        // slot 0 skips the read. An unpopulated
        // later slot would dereference a null MapWin*. Reproduced as-is.
        if (slot == 0 || MapWinTable[slot]->field_1DD74_ != 0) {
            terrain_fixup(MapWinTable[slot]);
        }
    }
    terrain_fixup(&WorldClimateWorldWin.mapWin_);
    terrain_fixup(MapWinTerrainFixupExtra2);
    terrain_fixup(&FarmMapWindow);
}

/*
Purpose: Step the receiver back to the subobject ??_GMainInterface@@UAEPAXI@Z
         expects, then forward unchanged.
// ORIGINAL: 0x0045F1F0 ??_GMainInterface@@WEEE@AEPAXI@Z 0x0045F1F0-0x0045F1FB BYTE_EXACT
// symbol    ??_EMainInterface@@WEEE@AEPAXI@Z
// CORRECTED from ??3MainInterface@@SAXPAXI@Z
//   11 bytes, `sub ecx, 0x444; jmp 0x0045F1C0` into
//   ??_GMainInterface@@UAEPAXI@Z, which executes `ret 4`; no stack
//   access and the receiver stays in ECX. `WEEE@` re-demangles to
//   adjustor{1092} and 1092 == 0x444, the constant subtracted
// size      11 bytes
// prototype
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: the forwarded call's
Status: Complete
*/

// THE SLOT INDEX go_reset (time.cpp) works against, defined here beside the
// table it indexes. The image seeds the dword at 0x0068A5CC with 0xffffffff -
// no window selected - so the initialiser folds straight into .data and pays
// no dynamic initialiser.
int MapWinSelectedSlot = -1;

/*
// ORIGINAL: 0x0046EB90 ?on_lose_mouse_capture@MapWin@@QAEXXZ 0x0046EB90-0x0046EB95 BYTE_EXACT
// symbol    ?on_lose_mouse_capture@MapWin@@UAEXXZ
// size      5 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      thunk
// flags     thunk;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/
// Losing the pointer cancels the go mode wholesale: the whole body is one
// tail `jmp go_reset` (E9 at 0x0046EB90), which a void function whose only
// statement is the void call compiles to under /O2 - the same sibcall merge
// game_close (game.cpp) documents.
void MapWin::on_lose_mouse_capture() {
    go_reset();
}

// ====================
// 0x0068FAF0 - THE TILE ARRAY'S ROW STRIDE, the multiplier in every
// `x + stride * y` tile lookup this file does. A named binding rather than an
// object, the same convention map.h's AltNatural uses for its neighbour at
// 0x0068FB4C: the image holds it at this fixed address and nothing here maps
// that storage.
static int *const MapRowStride = (int *)0x0068FAF0;

// 0x00939E5C - the main interface's own copy of the screen width, taken when
// the main window is sized: the setter at 0x0051D760 does
// `*this_screen_width_ = WinScreenWidth; this_dword_ = WinScreenWidth;` in
// that order, and `*g_00939EF0 = *this` copies it back out. Every reader
// tests it against 1000 to pick between the two fixed layouts - 550x300
// below, 704x384 at and above (0x00589760), 200x180 / 250x212 in
// InfoWin::UNK4 - which is 0.6875 of 800 and of 1024 alike, so the threshold
// is "is the desktop the wide one". config_popups now publishes the genuine
// IfaceScreenWidth storage declared in game.h.

// THE POINT-LIGHT TABLES. Three parallel signed-byte tables, one per tile
// flag, each indexed by the byte `AltDirectionTable` picks for the tile's
// code and the corner being lit - exactly the indexing the AltBonus* family
// below gets, so they are the same shape of table in the same family:
//
//     mov cl, byte ptr [eax + edx + 0x685684]   ; AltDirectionTable[code*5 + corner]
//     movsx edx, byte ptr [ecx + 0x685a24]      ; -> one of the three here
//
// `get_point_light` (0x00470790) picks between them on three low bits of the
// tile's `bit2` dword - 0x1, 0x2 and 0x200 - and nothing else in the tree
// names what those flags mean, so the names carry the selecting bit rather
// than a guessed terrain class. 0x00685A24 and 0x00685B60 are in the file
// image's .data and their bytes read as small signed deltas (-3..+3); the
// third, 0x007F6750, is past the bytes the file carries and is zero until
// something fills it.
static const signed char *const AltLightBit1 = (const signed char *)0x00685A24;
static const signed char *const AltLightBit2 = (const signed char *)0x00685B60;
static const signed char *const AltLightBit200 = (const signed char *)0x007F6750;

// Four mutable ocean-shade quads, shared across terrain calls. The renderer
// publishes each cell before passing these same slices to Texture painters,
// which may clamp the cells in place. Original storage is7F6644..7F6684.
// The last DWORD is also the first four bytes addressed by the biased
// corner-light lookup below; that consumer must observe this same owner.
// RECOVERY_OBJECT: v2 0x007F6644 0x007F6684 _OceanCornerShades 0x004632D0
static int OceanCornerShades[16];

// The initializer at 0x00470420 writes four consecutive 49-byte corner-light
// tables. This storage begins after the separately owned ocean-shade tail and
// ends at the observed exclusive boundary 0x007F674C.
// RECOVERY_OBJECT: v2 0x007F6688 0x007F674C ?MapLightingTable@@3PACA 0x00470420
signed char MapLightingTable[4 * 49];
static bool MapLightingTableInitialized;

static void fill_map_lighting_table() {
    for (int quadrant = 0; quadrant < 4; ++quadrant) {
        int index = 0;
        for (int row = -3; row <= 3; ++row) {
            for (int column = -3; column <= 3; ++column) {
                int x;
                int y;
                switch (quadrant) {
                case 0:
                    x = row;
                    y = column;
                    break;
                case 1:
                    x = -column;
                    y = row;
                    break;
                case 2:
                    x = -row;
                    y = -column;
                    break;
                default:
                    x = column;
                    y = -row;
                    break;
                }
                x *= 100;
                y *= 100;
                const int root = quick_root(x * x + y * y);
                int light = -3 * (x + y);
                if (root != 0) {
                    light /= root;
                }
                MapLightingTable[quadrant * 49 + index] =
                    static_cast<signed char>(light);
                ++index;
            }
        }
    }
    MapLightingTableInitialized = true;
}

static void ensure_map_lighting_table() {
    if (!MapLightingTableInitialized) {
        fill_map_lighting_table();
    }
}

// 0x007F6680 is the BIASED corner slope-light lookup base, not a proved
// allocation start. get_brighting reads each corner as
//
//     lea ebx, [eax*8] / sub ebx, eax            ; v * 7
//     movsx ebx, byte ptr [ebx + eax + 0x7f6680] ; [v*7 + w]
//
// where v and w are the two altitude-detail nibbles the visibility record
// holds for the two tiles that meet at that corner. `get_brighting`
// (0x00470550) indexes the four runs with the constants 0x7F6680, 0x7F66B1,
// 0x7F66E2 and 0x7F6713, so the stride is 0x31 = 49 = 7*7 and this one base
// serves all four.
//
// compute_lighting_table (470420) writes MapLightingTable at 7F6688..7F674C.
// For v,w in 1..7, 7*v+w = 8+7*(v-1)+(w-1): this lookup addresses that
// SAME generated table. The eight-byte bias does not prove another object.
// get_alt (47037A..47039C) stores (sum_four_tiers-4*recipient_tier+4)&15;
// the 1..7 domain needs all other corner tiers within one of the recipient.
// Without that invariant, reads can reach the ocean-shade tail at 7F6680,
// the four-byte gap before MapLightingTable, or beyond the generated table.
// Keep those exceptional cache states visible; do not silently clamp them.
static const signed char *const CornerLightTable = (const signed char *)0x007F6680;

static __forceinline signed char corner_light_value(int biased_index) {
    const int generated_index = biased_index - 8;
    if (biased_index >= 8 && biased_index < 8 + 4 * 49) {
        ensure_map_lighting_table();
        return MapLightingTable[generated_index];
    }
    return CornerLightTable[biased_index];
}

// 0x00685684 and the five tables behind it - THE ALTITUDE BONUS LOOKUP
// `get_alt` walks. 0x00685684 is the direction table (five bytes a direction,
// the first naming it and the four after it the neighbours it fans out to);
// the other five are the per-terrain bonus rows that direction's byte is
// indexed into. Named bindings, not objects, for the same reason as
// MapRowStride above: the image holds them at these fixed addresses and
// nothing here maps that storage.
// 0x00685EA0 / 0x00685EE0 - the two coordinate-stride tables the same walk
// adds to a tile's x and y as it steps outward, indexed by byte offset.
// THE VIRTUAL-BASE DISPLACEMENT the compiler lays MapWin down with: the
// vbtable it emits holds {0, 0x21A6C} (mapwin.h), so every handler the
// image enters on the GraphicWin base walks `entered - 0x21A6C` to get to
// the front, and every vbase-relative field sits at front + 0x21A6C + N.
// Held as a constant rather than read back out of the vbtable: the table
// is the compiler's, and reading it costs a this-pun at each site.
static const int32_t MapWinEnteredOffset = 0x21A6C;

// The receiver the image hands every Q-spelled MapWin handler: the GraphicWin
// virtual base, not the front. Named once here because four recovered bodies
// walk it once by name rather than re-deriving the displacement at each
// call site, where it reads as four unrelated puns instead of the one
// documented walk.
static char *entered_receiver(MapWin *window) {
    return reinterpret_cast<char *>(window) + MapWinEnteredOffset;
}

// The pinned image initializes both signed zoom-override words to -1.
// FontQueue::get_zoom at 0x005592D0 uses the first only when positive;
// otherwise it derives the scale from its explicit zoom argument.
// RECOVERY_OBJECT: v2 0x00691E6C 0x00691E70 ?MapWinCaptionScale@@3HA 0x005592D0
int MapWinCaptionScale = -1;
// RECOVERY_OBJECT: v2 0x00691E70 0x00691E74 ?MapWinCaptionDivisor@@3HA 0x005592D0
int MapWinCaptionDivisor = -1;
static const unsigned char *const AltStepY = (const unsigned char *)0x00685EE0;

// 0x0066932C / 0x00669330 - the USER32 import-table slots the tree reads as
// `int (__stdcall *)(int)` and calls with a virtual key:
//   GetKeyState(0x91)     (VK_SCROLL) - is Scroll Lock latched
//   GetAsyncKeyState(0x10)(VK_SHIFT)  - the high byte is the pressed bit
// Named for the import `iat_names.py` resolves each address to, because a
// fixed-address binding to an IAT slot is a function pointer and the tree has
// no other spelling for one.
static int *const IAT_GetKeyState = (int *)0x0066932C;
static int *const IAT_GetAsyncKeyState = (int *)0x00669330;

// ====================
// STORAGE gen_terrain_poly READS. Bounded source-owned tables below carry
// object mappings; the remaining address aliases still need their storage
// and producer closure. The terrain-band receivers themselves are the
// declared objects in texture.h / sprite.h.

// The shared raster light limits are owned by Texture (texture.h).
// Land rendering temporarily selects -4/+4 and restores -256/+256.

// Neighbour-mask texture and rotation pairs for moisture, jungle and fungus.
// 463623..463629 zero the masks; 463BCA/463BD2 and 463D82..463DE5
// only set bits 0..7. The six byte readers at 463EEB/463F64,
// 464EB6/464EEB and 46506C/4650C0 therefore bound this table to
// [685484,685684): 256 pairs, texture rows 0..14 and rotations 0..3.
// No direct writer or escaped pointer occurs in the pinned image references.
// Keep the original exceptional pairs for masks CA and DA ({11,2});
// geometric diagonal normalization would change them to {5,1}.
// RECOVERY_OBJECT: v2 0x00685484 0x00685684 _LandMoistureTable 0x004632D0
static const unsigned char LandMoistureTable[256][2] = {
    // Masks 00..0F.
    {0, 0}, {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {1, 0},
    {1, 3}, {1, 3}, {2, 0}, {2, 0}, {1, 3}, {1, 3}, {3, 0}, {3, 0},
    // Masks 10..1F.
    {0, 0}, {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {1, 0},
    {1, 3}, {1, 3}, {2, 0}, {2, 0}, {1, 3}, {1, 3}, {3, 0}, {3, 0},
    // Masks 20..2F.
    {1, 2}, {1, 2}, {4, 0}, {4, 0}, {1, 2}, {1, 2}, {4, 0}, {4, 0},
    {2, 3}, {2, 3}, {5, 0}, {5, 0}, {2, 3}, {2, 3}, {6, 0}, {6, 0},
    // Masks 30..3F.
    {1, 2}, {1, 2}, {4, 0}, {4, 0}, {1, 2}, {1, 2}, {4, 0}, {4, 0},
    {3, 3}, {3, 3}, {7, 0}, {7, 0}, {3, 3}, {3, 3}, {8, 0}, {8, 0},
    // Masks 40..4F.
    {0, 0}, {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {1, 0},
    {1, 3}, {1, 3}, {2, 0}, {2, 0}, {1, 3}, {1, 3}, {3, 0}, {3, 0},
    // Masks 50..5F.
    {0, 0}, {0, 0}, {1, 0}, {1, 0}, {0, 0}, {0, 0}, {1, 0}, {1, 0},
    {1, 3}, {1, 3}, {2, 0}, {2, 0}, {1, 3}, {1, 3}, {3, 0}, {3, 0},
    // Masks 60..6F.
    {1, 2}, {1, 2}, {4, 0}, {4, 0}, {1, 2}, {1, 2}, {4, 0}, {4, 0},
    {2, 3}, {2, 3}, {5, 0}, {5, 0}, {2, 3}, {2, 3}, {6, 0}, {6, 0},
    // Masks 70..7F.
    {1, 2}, {1, 2}, {4, 0}, {4, 0}, {1, 2}, {1, 2}, {4, 0}, {4, 0},
    {3, 3}, {3, 3}, {7, 0}, {7, 0}, {3, 3}, {3, 3}, {8, 0}, {8, 0},
    // Masks 80..8F.
    {1, 1}, {1, 1}, {2, 1}, {3, 1}, {1, 1}, {1, 1}, {2, 1}, {3, 1},
    {4, 3}, {4, 3}, {5, 1}, {6, 1}, {4, 3}, {4, 3}, {7, 1}, {8, 1},
    // Masks 90..9F.
    {1, 1}, {1, 1}, {2, 1}, {3, 1}, {1, 1}, {1, 1}, {2, 1}, {3, 1},
    {4, 3}, {4, 3}, {5, 1}, {6, 1}, {4, 3}, {4, 3}, {7, 1}, {8, 1},
    // Masks A0..AF.
    {2, 2}, {2, 2}, {5, 2}, {7, 2}, {2, 2}, {2, 2}, {5, 2}, {7, 2},
    {5, 3}, {5, 3}, {9, 0}, {10, 0}, {5, 3}, {5, 3}, {10, 3}, {11, 0},
    // Masks B0..BF.
    {2, 2}, {2, 2}, {5, 2}, {7, 2}, {2, 2}, {2, 2}, {5, 2}, {7, 2},
    {6, 3}, {6, 3}, {10, 2}, {12, 0}, {6, 3}, {6, 3}, {11, 3}, {13, 0},
    // Masks C0..CF.
    {1, 1}, {1, 1}, {2, 1}, {3, 1}, {1, 1}, {1, 1}, {2, 1}, {3, 1},
    {4, 3}, {4, 3}, {11, 2}, {6, 1}, {4, 3}, {4, 3}, {7, 1}, {8, 1},
    // Masks D0..DF.
    {1, 1}, {1, 1}, {2, 1}, {3, 1}, {1, 1}, {1, 1}, {2, 1}, {3, 1},
    {4, 3}, {4, 3}, {11, 2}, {6, 1}, {4, 3}, {4, 3}, {7, 1}, {8, 1},
    // Masks E0..EF.
    {3, 2}, {3, 2}, {6, 2}, {8, 2}, {3, 2}, {3, 2}, {6, 2}, {8, 2},
    {7, 3}, {7, 3}, {10, 1}, {11, 1}, {7, 3}, {7, 3}, {12, 3}, {13, 1},
    // Masks F0..FF.
    {3, 2}, {3, 2}, {6, 2}, {8, 2}, {3, 2}, {3, 2}, {6, 2}, {8, 2},
    {8, 3}, {8, 3}, {11, 2}, {13, 2}, {8, 3}, {8, 3}, {13, 3}, {14, 0},
};

// Sensor-distance origin shared with Console::new_base: 4CFF65/4CFF6B
// publish X/Y before redraw, and 4CFF9E/4CFFA4 reset each signed DWORD
// to -99. The original initial storage also contains -99 for each.
// RECOVERY_OBJECT: v2 0x00685478 0x0068547C ?MapFocusX@@3HA 0x004CF7C0
int MapFocusX = -99;
// RECOVERY_OBJECT: v2 0x0068547C 0x00685480 ?MapFocusY@@3HA 0x004CF7C0
int MapFocusY = -99;

// Original 46479D..464B62 reads signed receiver/rotation pairs. Only a
// texture of -1 suppresses drawing: Crater row 31 uses texture 2 with
// rotation -1, which the corner mask turns into rotation 3.
struct LandmarkTextureChoice {
    signed char texture;
    signed char rotation;
};
static_assert(sizeof(LandmarkTextureChoice) == 2, "landmark pair stride");

// The bounded read domain is 49 rows, not a claim about original allocator
// metadata. Crater/Mesa enumerate 0..48 (5C5EBD/5C8113). Mount itself creates
// nine codes, but explicit Mesa placement bypasses near_landmark at 5C7F50..67;
// bit2_set preserves Mount's flag and code_set overwrites the shared code.
// Overlapping row 48 is reachable, so Mount's sentinel tail must remain.
// Monsoon overlap can retain these flags while publishing higher codes:
// row 49 reaches the old alignment bytes, and row 119 another data owner.
// Deliberate, user-approved behavior change: negative or out-of-table codes
// skip only that landmark overlay. Never read a neighboring object, clamp
// to a different row, or omit the remaining terrain work. This is not an
// original-byte equivalence claim for those overlapping/loaded/edited codes.
// No direct writers or pointer escapes were found for these data ranges.
// Irregular pair initializers are materialized only in ignored build context.
// RECOVERY_OBJECT: v2 0x00685A98 0x00685AFA _MountPlanetTable 0x004632D0
static const LandmarkTextureChoice MountPlanetTable[49] = {
#include "terrain_mount_planet.inc"
};
// RECOVERY_OBJECT: v2 0x00685AFC 0x00685B5E _SunnyMesaTable 0x004632D0
static const LandmarkTextureChoice SunnyMesaTable[49] = {
#include "terrain_sunny_mesa.inc"
};
// RECOVERY_OBJECT: v2 0x00685BD4 0x00685C36 _GarlandCraterTable 0x004632D0
static const LandmarkTextureChoice GarlandCraterTable[49] = {
#include "terrain_garland_crater.inc"
};

// OceanDetailShade is the shared load_cycling-owned table in game.cpp.
// Unworked tiles use this separately owned scalar read at 464148 instead
// of get_point_light's value; it is not part of that cycling table.
// RECOVERY_OBJECT: v2 0x00686354 0x00686358 _UnworkedCornerLight 0x004632D0
static int UnworkedCornerLight = 2;

// Four land triangles, twelve (x,y) texel coordinates: 463E99..463F8B
// visits the full array. Overlay base 686098 is vertex 1 of this SAME
// allocation; 464ED2/465088 select vertex 3*rotation+1, with rotation&3.
// The dune call at 464254..464353 borrows an unrotated triangle slice.
// Keep that shared lifetime and mutable-pointer API, although the original
// 6233C0/61B1F0 consumers only read UVs through 61FC90. The asymmetric
// 27/28 centres and 1/2/53/54 edge coordinates are intentional data.
// RECOVERY_OBJECT: v2 0x00686090 0x006860F0 _LandCornerUVs 0x004632D0
static Vert LandCornerUVs[12] = {
    {27, 28}, {0, 55}, {0, 1},
    {27, 27}, {0, 0}, {54, 0},
    {28, 27}, {55, 1}, {55, 53},
    {28, 29}, {55, 55}, {2, 55},
};

// Four edge quads. 463FB1 increments the byte offset BEFORE loads using
// displacement 6860E8, so the first vertex is 6860F0, not land vertex 11.
// The offset terminates at 0x80 (464012): exactly sixteen vertices.
// RECOVERY_OBJECT: v2 0x006860F0 0x00686170 _EdgeMidUVs 0x004632D0
static const Vert EdgeMidUVs[16] = {
    {27, 27}, {0, 27}, {0, 0}, {27, 0},
    {28, 27}, {28, 0}, {55, 0}, {55, 27},
    {28, 28}, {55, 28}, {55, 55}, {28, 55},
    {27, 28}, {27, 55}, {0, 55}, {0, 28},
};

// Four ocean quads: 4645C4/CA read pairs, 4645E7 advances eight bytes,
// and the four-by-four walk ends at 464706. No direct table writers occur
// in the original references; these are texel coordinates, not pointers.
// RECOVERY_OBJECT: v2 0x00685CA8 0x00685D28 _OceanCornerUVs 0x004632D0
static const Vert OceanCornerUVs[16] = {
    {27, 27}, {27, 55}, {0, 55}, {0, 0},
    {27, 27}, {27, 0}, {55, 0}, {55, 55},
    {27, 27}, {55, 27}, {55, 55}, {27, 55},
    {27, 27}, {0, 27}, {0, 0}, {27, 0},
};

// 0x00684730 - the 53-entry coast-mask table draw_coast's remap comes from
// (stride 8, one row per corner code). Values above 1 select bytes at
// TerrainCoastPalette[value + 2]; code 7 repeats palette[7..4] three times.
// load_terrain publishes this shared twelve-byte owner. The deep-radar walk
// uses the existing Chassis[].triad member, not a separate character table.
// 465379 selects a column using the three-bit coast mask, then advances
// eight bytes for each of 53 remap entries. Values above 1 select the
// separately mutable coast palette; this table does not own that palette.
// RECOVERY_OBJECT: v2 0x00684730 0x006848D8 _CoastMaskTable 0x004632D0
static const unsigned char CoastMaskTable[53][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 1, 0, 1, 0, 1, 0, 1},
    {0, 1, 0, 1, 0, 1, 1, 1},
    {0, 0, 0, 0, 1, 1, 1, 1},
    {0, 0, 0, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 1},
    {0, 5, 0, 5, 0, 5, 0, 5},
    {0, 4, 0, 4, 0, 4, 0, 4},
    {0, 3, 0, 3, 0, 3, 0, 3},
    {0, 2, 0, 1, 0, 2, 0, 2},
    {0, 0, 0, 0, 5, 5, 5, 5},
    {0, 0, 0, 0, 4, 4, 4, 4},
    {0, 0, 0, 0, 3, 3, 3, 3},
    {0, 0, 0, 0, 2, 2, 2, 2},
    {0, 0, 0, 0, 0, 0, 0, 5},
    {0, 0, 0, 0, 0, 0, 0, 4},
    {0, 0, 0, 0, 0, 0, 0, 3},
    {0, 0, 0, 0, 0, 0, 0, 2},
    {0, 5, 0, 5, 0, 5, 0, 1},
    {0, 4, 0, 4, 0, 4, 0, 1},
    {0, 3, 0, 3, 0, 3, 0, 1},
    {0, 2, 0, 2, 0, 2, 0, 1},
    {0, 0, 0, 0, 5, 5, 5, 1},
    {0, 0, 0, 0, 4, 4, 4, 1},
    {0, 0, 0, 0, 3, 3, 3, 1},
    {0, 0, 0, 0, 2, 2, 2, 1},
    {0, 0, 0, 5, 0, 0, 5, 1},
    {0, 0, 0, 4, 0, 0, 4, 1},
    {0, 0, 0, 3, 0, 0, 3, 1},
    {0, 0, 0, 2, 0, 0, 2, 1},
    {0, 1, 0, 1, 0, 1, 5, 1},
    {0, 1, 0, 1, 0, 1, 4, 1},
    {0, 1, 0, 1, 0, 1, 3, 1},
    {0, 1, 0, 1, 0, 1, 2, 1},
    {0, 0, 0, 5, 1, 1, 1, 1},
    {0, 0, 0, 4, 1, 1, 1, 1},
    {0, 0, 0, 3, 1, 1, 1, 1},
    {0, 0, 0, 2, 1, 1, 1, 1},
    {0, 5, 0, 1, 0, 5, 1, 1},
    {0, 4, 0, 1, 0, 4, 1, 1},
    {0, 3, 0, 1, 0, 3, 1, 1},
    {0, 2, 0, 1, 0, 2, 1, 1},
    {0, 0, 0, 1, 5, 5, 1, 1},
    {0, 0, 0, 1, 4, 4, 1, 1},
    {0, 0, 0, 1, 3, 3, 1, 1},
    {0, 0, 0, 1, 2, 2, 1, 1},
    {0, 5, 0, 1, 2, 0, 1, 1},
    {0, 4, 0, 1, 3, 0, 1, 1},
    {0, 3, 0, 1, 4, 0, 1, 1},
    {0, 5, 5, 1, 5, 0, 1, 1},
    {0, 4, 4, 1, 4, 0, 1, 1},
    {0, 2, 0, 1, 0, 3, 1, 1},
    {0, 0, 0, 1, 5, 3, 1, 1},
};

// Four triangles around the tile centre: left/top, top/right,
// right/bottom, bottom/left. Corner IDs are centre=0, left=1, top=2,
// right=3, bottom=4. The original normal walk (463E68..463F85) visits
// twelve dwords per table; the wireframe walk (466ADC..466BAF) independently
// visits four groups of three. These are real tables, not image pointers.
// RECOVERY_OBJECT: v2 0x00685F20 0x00685F50 _LandCornerXStep 0x004632D0
static const int LandCornerXStep[12] = {
    0, -2, 0,  0, 0, 2,  0, 2, 0,  0, 0, -2
};
// RECOVERY_OBJECT: v2 0x00685F50 0x00685F80 _LandCornerYStep 0x004632D0
static const int LandCornerYStep[12] = {
    0, 0, -2,  0, -2, 0,  0, 0, 2,  0, 2, 0
};
// RECOVERY_OBJECT: v2 0x00685F80 0x00685FB0 _LandCornerId 0x004632D0
static const int LandCornerId[12] = {
    0, 1, 2,  0, 2, 3,  0, 3, 4,  0, 4, 1
};

// THE RENDERER'S SCRATCH, in .data immediately below MapWinTable:
// 0x007D3C08 - the four primary corner vertices as eight ints (the image
// reads the y half through the alias 0x007D3C0C, one int up);
// 0x007D3BE8 - the eight-int copy the tile under the map landmark keeps;
// 0x007D3AF0 - the sixteen ocean corner vertices (four per corner);
// 0x007F6644 - the sixteen shared ocean shades defined beside their biased
// lighting consumer above. gen_terrain_poly fills these after each
// alt_get_ocean_detail call; the helper itself returns only the table index.
// The four-by-four walk writes x/y at 46459E/4645AC and passes the
// same four-vertex slices at 46466D/46469A/4646C6. All observed original
// references belong to this renderer; 7D3B70 is a separate scalar.
// Keep static zero-initialized storage, not a fresh per-call array.
// RECOVERY_OBJECT: v2 0x007D3AF0 0x007D3B70 _OceanCornerVerts 0x004632D0
static Vert OceanCornerVerts[16];

// Terrain sprite draws read the unsigned palette key at original member +8.
// Use Sprite's actual member rather than walking its object representation.
static inline unsigned char sprite_transparent(const Sprite &sprite) {
    return sprite.transparent_index();
}

// The worked-tile scan's missing-owner case is an original indexing bug:
// 463927 sets owner=-1, then 463931-463943 reads a treaty bit with that
// owner as its row. In the original layout the read aliases vehicle state,
// not a diplomacy record. For the scan's valid faction domain 1..7, retain
// the observed low bit through the actual owning fields. These are not
// meaningful faction/vehicle relationships or a dummy "faction -1" row.
static bool terrain_missing_owner_treaty_bit(int faction) {
    switch (faction) {
    case 1: return (Vehs[1895].waypoint_y[0] & 1) != 0;
    case 2: return (Vehs[1895].waypoint_y[2] & 1) != 0;
    case 3: return (Vehs[1895].morale & 1) != 0;
    case 4: return (Vehs[1895].moves_expended & 1) != 0;
    case 5: return (Vehs[1895].probe_action & 1) != 0;
    case 6: return (Vehs[1895].next_veh_id_stack & 1) != 0;
    case 7: return (Vehs[1896].x & 1) != 0;
    }
    // Behavior outside factions 1..7 is not recovered.
    return false;
}

/*
Purpose: Render one map tile's terrain polygon stack - the base land or
         ocean fill with per-corner lighting, landmark overlays, farms,
         jungle, fungus and forest, rivers and coasts, roads and magtubes,
         improvement and wreck sprites, territory borders, wireframes, grid
         overlays and the editor's numeric readouts.
         The two compiled callers (gen_tile 0x00468C73 and gen_map's
         terrain scan 0x00469E47) push ten words and clean up 0x28 bytes
         themselves, so the body is the FREE __cdecl function those sites
         spell; the catalogued member mangle below remains the identity of
         record for the span (mapwin.h documents the reinterpretation; the
         former pending_bodies.cpp forwarder was removed when this body
         landed, journal 64).

         RECEIVER EVIDENCE (journal 55, terrain-poly-receivers-20260921,
         re-verified per site against the pinned image): every one of the
         44 blitter calls is __thiscall - each callee ends `ret imm16` and
         no caller does `add esp` after it - and every ECX receiver traces
         to a fixed terrain-band global from texture.h / sprite.h or to
         the incoming Buffer* a1 (the three Buffer::line sites). Zero
         receivers are MapWin fields or stack locals; the MapWin* a2
         supplies lighting through get_alt/get_point_light, its visibility
         record through owned_, and its font_b_ for the readouts.

         THE EXTENT CAVEAT, RESOLVED (journal 56's mandatory pre-work):
         g_ROCKY_TEXTURES stays [4] and g_FARM_TEXTURES stays [9].
         load_terrain fills the rocky band with exactly four extracts -
         bases 0x76E9E0 and 0x76EA50 (outer bound `cmp edi, 0x76EAC0` at
         0x451D67), each extracting two slots 0xE0 apart - and the farm
         band with exactly nine (three inner extracts a row from 0x799238,
         outer `cmp esi, 0x799628` at 0x45213F = 9 * 0x70). The renderer's
         rocky index `variant + 2 * rockiness - 2` stays inside 0..3 for
         the real rockiness values 1..2 because Random::get(0, 2) can never
         return 2: the recovered 0x00625770 body computes
         ((range * (seed & 0xFFFF)) >> 16) + min, and with range == 2 the
         product tops at 0x1FFFE, which shifts down to 1. The farm index
         `aridity + 3 * variant` is bounded by 2 + 3 = 5 with this exclusive
         roll, inside the nine filled slots.

         THE THREE NEIGHBOUR-MASK LOOPS the scaffold carried empty,
         restored from the image: 0x464E24-0x464EA2 walks the four
         DIAGONAL neighbours (RiverStepX/Y) collecting a
         land-with-base-radius corner mask into the dead a5 parameter
         slot - the shipped binary then indexes the jungle textures with
         the EARLIER 8-neighbour base mask at 0x464EB6, so the a5 mask is
         dead from 0x464EA4 on; the loop stays because its reads are the
         image's own behaviour. 0x464F5B-0x464FD6 collects the four-corner
         fungus mask that selects g_FOREST_TEXTURES[mask], and
         0x4654EC-0x46555F the four-corner river mask (neighbour
         tile->bit & BIT_RIVER) that selects g_RIVER_TEXTURES[mask].
// ORIGINAL: 0x004632D0 ?gen_terrain_poly@MapWin@@QAAHPAUBuffer@@PAUMapWin@@HHHHHHHH@Z 0x004632D0-0x00466C92;0x00655920-0x00655932 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original4632D0..466C92 and unwind655920..655932; the retired seed git:c4688c7b:src/unrecovered/004632d0.cpp records the same identity and spans, including historical missing-receiver analysis (its durable facts live in the TRIED note below; the seed file's deletion and this body's landing are one change).", "Original4712D0..471316 explicitly converts its nullable MapWin argument to Buffer before forwarding ten arguments to this renderer. Original469190..469718 and40F0F0..40FCB3 also supply explicit buffer/window arguments.", "Original texture/sprite call sites require genuine object receivers; declaring Random local_rand gives the compiler ownership of constructor, destructor and exception cleanup. Original464D12..464D21 copies four shared corner vertices for the map-radius and scroll consumers."], "reason": "The source-owned terrain renderer is a cdecl free function with explicit Buffer and nullable MapWin arguments, not the catalogue member spelling. It owns the compiled candidate while the original seed remains provenance; storage, lifecycle and dependent-renderer discoveries remain unfinished recovery obligations."}
// symbol    ?gen_terrain_poly@@YAHPAVBuffer@@PAVMapWin@@HHHHHHHH@Z
// size      14804 bytes
// prototype int (__cdecl ?gen_terrain_poly@MapWin@@QAAHPAUBuffer@@PAUMapWin@@HHHHHHHH@Z)(Buffer*, MapWin*, int, int, int, int, int, int, int, int)
// callers   7   call targets   38
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00462190 0x0046FE70 0x00470790 0x004E3A50 0x004E3EF0 0x004E6E50 0x0050BA00 0x00591B50 0x00592030 0x00592140 0x005BF010 0x005BF1F0 0x005BFE90 0x005C4FD0 0x005DA680 0x005DAC70 0x005DACB0 0x005DD130 0x005E15F0 0x005E4B9A 0x005E5833 0x0061A760 0x0061B1F0 0x0061C090 0x0061E540 0x00622DB0 0x006233C0 0x006239E0 0x00623F80 0x00624670 0x00625730 0x00625740 0x00625750 0x00625770 0x00644F3A 0x006453E0 0x00645470 0x0064FC88
// TRIED (seed git:c4688c7b:src/unrecovered/004632d0.cpp, retired
//            atomically by this body): the 113 fixed globals it reads are
//            named in sprite.h /
//            texture.h (the terrain Sprite/Texture band
//            0x0075B098..0x007AD1B0, ~50 of them the +8
//            `&name[k].cTransparentIndex_` companion of a named array);
//            CornerOffsetX/Y, CornerLightTable, AltLightBit1/2/200 and
//            AltDirectionTable are bound in map.h and above in this file;
//            the rest are the named bindings above. Its BLOCKED note's
//            remaining unnamed .rdata/BSS list (684730, 685478..,
//            685A98.., 686090.., 6861F0, 686354, 68E564.., 952832..,
//            96C9E1, 97D04A, 9A6490.., 9AB88C, 9BB548) is discharged by
//            the bounded RECOVERY_OBJECT tables above this body, not by
//            fresh fixed-address bindings. Its BLOCKED note's
//            56-call count was corrected to the machine's 44. The SEH
//            frame and the Random ctor/dtor calls are the compiler's, from
//            declaring `Random local_rand;` - the note's own TRIED result.
*/
int __cdecl gen_terrain_poly(Buffer *buffer, MapWin *window, int a3, int a4,
                             int a5, int a6, int a7, int a8, int a9,
                             int a10) {
    uint8_t coast_remap[0x35];
    uint8_t territory_mask[10];
    char numbuf[80];
    Vert rocky_uv[12];
    Vert ocean_uv[16];
    Vert edge_vertices[16];
    Vert edge_uv[16];
    Vert land_uv[12];
    Vert land_vertices[12];
    int triangle_light[3];
    uint32_t local_11c;
    int coastal_mask[4];
    int result;
    int local_c4;
    uint32_t local_c0;
    uint32_t local_bc;
    int local_b8[4];
    int local_a8;
    int local_a4;
    int local_a0;
    int local_9c;
    int local_98;
    int local_94;
    int local_90;
    uint32_t local_8c;
    int local_88;
    uint32_t local_84;
    int local_80;
    int local_78;
    int local_70;
    Vert overlay_uv[4];
    uint32_t territory_faction;
    Map *road_neighbor;
    uint32_t skip_terrain;
    int focus_y;
    uint32_t local_44;
    uint32_t local_40;
    uint32_t local_38;
    uint32_t local_34;
    uint32_t local_30;
    uint32_t local_2c;
    uint32_t local_28;
    uint32_t local_24;
    uint32_t local_20;
    int local_1c;
    uint32_t local_18;
    int local_14;
    uint32_t local_48b;
    int local_30b;
    uint32_t local_34b;
    Random local_rand;
    int crop_diff = 0;
    int iVar5;
    int iVar6;
    int iVar7;
    int iVar13;
    int iVar19;
    uint32_t uVar12;
    uint32_t uVar16;
    uint32_t uVar17;
    int32_t uVar15;
    int *puVar8;
    uint8_t *pbVar18;
    bool bVar23;
    uint8_t bVar10;
    uint8_t bVar11;
    char cVar4;
    unsigned int sVar9;
    short sVar2;
    uint8_t uVar1;
    uint32_t uVar16b;

    // -0x24 in the image: the viewing faction's tile-bit view (tile->bit
    // for an observer or an explored map. The physical faction-zero slot
    // is bit2; the seven ordinary faction slots are bit_visible[a4-1].
    uint32_t tile_bits;
    (void)uVar16b;
    (void)uVar1;
    (void)sVar2;
    (void)uVar16;
    (void)uVar17;
    (void)cVar4;

    if (a8 < 0 || MapLatitudeBounds <= a8 || a7 < 0 ||
        MapLongitudeBounds <= a7) {
        return 0;
    }
    local_14 = a7 >> 1;
    iVar5 = *MapRowStride * a8 + local_14;
    Map *const tile = &map_tiles()[iVar5];
    uint32_t *const tile_bit2_ptr = &tile->bit2;
    local_34 = tile->bit2;
    result = 0;
    if (window == &WorldClimateWorldWin.mapWin_ && (a6 & 0x80000000) == 0 &&
        (local_34 & 0x400000) == 0) {
        return 1;
    }
    local_rand.reseed(static_cast<uint32_t>(reinterpret_cast<uintptr_t>(tile)));
    pbVar18 = reinterpret_cast<uint8_t *>(tile);
    local_18 = (uint32_t)((tile->climate & 0xe0) < 0x60);
    if (local_18 == 0) {
        TextureLightFloor = 0xfffffffc;
        TextureLightCeiling = 4;
    }
    if ((local_34 & 0x103) != 0) {
        local_94 = static_cast<int32_t>(tile->bit2) >> 24;
    } else {
        local_94 = 0;
    }
    if (MapWinCaptionScale < 1) {
        iVar5 = (a3 * 5 + 0x50) * 10;
        iVar5 = (int)(iVar5 + (iVar5 >> 0x1f & 0xfU)) >> 4;
    } else {
        iVar5 = MapWinCaptionScale * 0x32 / MapWinCaptionDivisor;
    }
    local_98 = (int)(iVar5 + 1 + (iVar5 + 1 >> 0x1f & 3U)) >> 2;
    local_78 = local_98 * 2;
    if (window == 0) {
        local_44 = 0;
    } else {
        iVar5 = window->get_alt(a7, a8, 4);
        iVar19 = window->get_alt(a7, a8, 3);
        iVar6 = window->get_alt(a7, a8, 2);
        iVar7 = window->get_alt(a7, a8, 1);
        local_44 = iVar5 + iVar19 + iVar6 + iVar7 >> 2;
    }
    a10 = a10 + (int)local_44;
    territory_faction = (uint32_t)a4;
    if ((a4 < 0) &&
        (bVar10 = tile->val3, territory_faction = bVar10 & 7,
         (bVar10 & 7) == 0)) {
        territory_faction = (uint32_t)ConsoleGlobal.faction_id_;
    }
    iVar5 = *MapRowStride * a8 + local_14;
    pbVar18 = reinterpret_cast<uint8_t *>(&map_tiles()[iVar5]);
    local_84 = (tile->climate & 0x18) >> 3;
    local_8c = (uint32_t)(tile->climate >> 5);
    local_38 = (uint32_t)(tile->val3 >> 6);
    if (a4 < 0 || (PlayersData[a4].flags & 0x200) != 0) {
        local_24 = tile->bit;
    } else {
        local_24 = a4 == 0 ? tile->bit2 : tile->bit_visible[a4 - 1];
    }
    tile_bits = local_24;
    if (a3 < -6 || (a6 & 0x80000000) != 0) {
        local_90 = 1;
    } else {
        local_90 = 0;
    }
    local_11c = a6 & 0x1000000;
    bVar10 = (uint8_t)a4;
    if ((a6 & 0x1000000) == 0) {
        // At 4635C4, observer -1's flags lookup aliases the high byte of
        // Vehs[1895].x in the original layout. Preserve that accidental
        // dependency by its real field, not an out-of-bounds player row.
        // Other out-of-range faction values have no recovered contract.
        if ((((GameDataStateStorage.fields_.game_rules_ & 0x100) != 0) &&
             ((GameDataStateStorage.fields_.game_state_ & 0x80) == 0) &&
             ((a4 == -1 ? (Vehs[1895].x & 0x200)
                        : (PlayersData[a4].flags & 0x200)) == 0) &&
             ((tile->visibility & (uint8_t)(1u << (bVar10 & 0x1f))) == 0))) {
            local_84 = 0;
            local_38 = 0;
            local_24 = local_24 & 0xffffffdf;
            tile_bits = local_24;
            local_70 = 1;
            local_1c = 0;
            uVar12 = tile->bit2 | 0x200000;
            goto LAB_00463603;
        }
        local_70 = 0;
        if (((GameDataStateStorage.fields_.game_state_ & 0x80) != 0) || a4 < 1 ||
            ((GameDataStateStorage.fields_.game_more_preferences_ & 1) == 0 || a3 < -7)) {
            goto LAB_00463589;
        }
        local_1c = 0;
        local_40 = 0;
        do {
            local_20 = RadiusOffsetX[local_40 >> 2] + a7;
            if ((MapIsFlat & 1) == 0) {
                if ((int)local_20 < 0) {
                    local_20 = local_20 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= (int)local_20) {
                    local_20 = local_20 - MapLongitudeBounds;
                }
            }
            iVar5 = RadiusOffsetY[local_40 >> 2] + a8;
            if (iVar5 > -1 && iVar5 < MapLatitudeBounds &&
                (int)local_20 > -1 && (int)local_20 < MapLongitudeBounds) {
                if ((int)local_40 < 0x24) {
                    iVar19 = *MapRowStride * iVar5 + ((int)local_20 >> 1);
                    Map *const ntile = &map_tiles()[iVar19];
                    if ((ntile->bit & 3) == 0) {
                        uVar12 = 0xffffffff;
                    } else {
                        uVar12 = ntile->val2 & 0xf;
                        if (7 < uVar12) {
                            uVar12 = 0xffffffff;
                        }
                    }
                    if (((int)uVar12 == a4) ||
                        ((VehDrawPositionX > -1 &&
                          Vehs[VehDrawPositionVehicle].faction_id ==
                              (uint8_t)a4 &&
                          VehDrawPositionX == (int)local_20 &&
                          VehDrawPositionY == iVar5))) {
                        goto LAB_00463b55;
                    }
                    // The image does not guard the no-owner -1 column:
                    // 4637B9 then reads the preceding +0x14 member.
                    const uint32_t treaty = (int)uVar12 == -1
                        ? (uint32_t)PlayersData[a4].last_turn_new_base
                        : PlayersData[a4].diplo_treaties[uVar12];
                    if ((treaty & 1) != 0) {
                        for (iVar19 = veh_at(local_20, iVar5); iVar19 > -1;
                             iVar19 = Vehs[iVar19].next_veh_id_stack) {
                            if (Vehs[iVar19].faction_id == (uint8_t)a4) {
                                goto LAB_00463b55;
                            }
                            if (iVar19 < 0) {
                                break;
                            }
                        }
                        if (local_1c != 0) {
                            goto LAB_0046358c;
                        }
                    }
                }
                uVar12 = local_20;
                iVar6 = is_sensor(local_20, iVar5);
                iVar19 = VehDrawPositionVehicle;
                iVar7 = (int)uVar12 >> 1;
                if (iVar6 != 0 &&
                    map_tiles()[*MapRowStride * iVar5 + iVar7].territory ==
                        a4) {
                    goto LAB_00463b55;
                }
                iVar6 = *MapRowStride * iVar5 + iVar7;
                Map *const stile = &map_tiles()[iVar6];
                if ((stile->bit & 1) == 0) {
                    uVar16b = 0xffffffff;
                } else {
                    uVar16b = stile->val2 & 0xf;
                    if (7 < uVar16b) {
                        uVar16b = 0xffffffff;
                    }
                }
                if (((int)uVar16b == a4) ||
                    ((VehDrawPositionX == uVar12 &&
                      VehDrawPositionY == iVar5) &&
                     (Vehs[VehDrawPositionVehicle].faction_id ==
                          (uint8_t)a4 &&
                      has_abil(Vehs[iVar19].proto_id, ABL_DEEP_RADAR) != 0 &&
                      (Chassis[VehPrototypes[Vehs[iVar19].proto_id].chassis_id].triad == '\x02' ||
                       local_18 ==
                           (uint32_t)(Chassis[VehPrototypes[Vehs[iVar19].proto_id].chassis_id].triad == '\x01'))))) {
                    goto LAB_00463b55;
                }
                Map *const vtile = &map_tiles()[iVar6];
                if ((vtile->bit & 2) == 0) {
                    uVar12 = 0xffffffff;
                } else {
                    uVar12 = vtile->val2 & 0xf;
                    if (7 < uVar12) {
                        uVar12 = 0xffffffff;
                    }
                }
                if ((((int)uVar12 == a4) ||
                     ((int)uVar12 == -1
                          ? terrain_missing_owner_treaty_bit(a4)
                          : (PlayersData[uVar12].diplo_treaties[a4] & 1) != 0)) &&
                    (iVar5 = veh_at(local_20, iVar5), iVar5 > -1)) {
                    sVar2 = Vehs[iVar5].prev_veh_id_stack;
                    while (sVar2 > -1) {
                        iVar5 = sVar2;
                        sVar2 = Vehs[iVar5].prev_veh_id_stack;
                    }
                    if (iVar5 > -1) {
                        while ((Vehs[iVar5].faction_id != (uint8_t)a4 ||
                                has_abil(Vehs[iVar5].proto_id,
                                         ABL_DEEP_RADAR) == 0) ||
                               (Chassis[VehPrototypes[Vehs[iVar5].proto_id].chassis_id].triad != '\x02' &&
                                local_18 !=
                                    (uint32_t)(Chassis[VehPrototypes[Vehs[iVar5].proto_id].chassis_id].triad == '\x01'))) {
                            if (iVar5 < 0) {
                                goto LAB_00463a11;
                            }
                            iVar5 = Vehs[iVar5].next_veh_id_stack;
                            if (iVar5 < 0) {
                                goto LAB_00463a11;
                            }
                        }
                        local_1c = 1;
                    }
                }
            }
LAB_00463a11:
            local_40 = local_40 + 4;
        } while ((int)local_40 < 100);
        if (local_1c == 0) {
            if (ExpansionEnabled != 0) {
                iVar5 = 0;
                do {
                    iVar6 = RadiusOffsetY[iVar5 >> 2];
                    iVar19 = RadiusOffsetX[iVar5 >> 2];
                    iVar7 = (iVar6 - iVar19) >> 1;
                    iVar13 = (iVar19 + iVar6) >> 1;
                    if (iVar7 * iVar7 + iVar13 * iVar13 < 0x19) {
                        iVar19 = iVar19 + a7;
                        if ((MapIsFlat & 1) == 0) {
                            if (iVar19 < 0) {
                                iVar19 = iVar19 + MapLongitudeBounds;
                            } else if (MapLongitudeBounds <= iVar19) {
                                iVar19 = iVar19 - MapLongitudeBounds;
                            }
                        }
                        iVar6 = iVar6 + a8;
                        if (iVar6 > -1 && iVar6 < MapLatitudeBounds &&
                            iVar19 > -1 && iVar19 < MapLongitudeBounds) {
                            iVar7 = *MapRowStride * iVar6 + (iVar19 >> 1);
                            Map *const ntile = &map_tiles()[iVar7];
                            if ((ntile->bit & 1) == 0) {
                                uVar12 = 0xffffffff;
                            } else {
                                uVar12 = ntile->val2 & 0xf;
                                if (7 < uVar12) {
                                    uVar12 = 0xffffffff;
                                }
                            }
                            if ((int)uVar12 == a4) {
                                iVar19 = base_at(iVar19, iVar6);
                                int fac_off;
                                int fac_mask;
                                bitmask(0x29, &fac_off, &fac_mask);
                                if ((Bases[iVar19]
                                         .facilities_built[fac_off] &
                                     (uint8_t)fac_mask) != 0) {
                                    goto LAB_00463b55;
                                }
                            }
                        }
                    }
                    iVar5 = iVar5 + 4;
                } while (iVar5 < 0x144);
            }
            local_84 = 0;
            local_38 = 0;
            local_24 = local_24 & 0xffffffdf;
            tile_bits = local_24;
            local_70 = 1;
            local_1c = 0;
            uVar12 = tile->bit2 | 0x200000;
            goto LAB_00463603;
        }
    } else {
        local_84 = 0;
        local_38 = 0;
        local_24 = 0;
        tile_bits = 0;
        local_70 = 1;
LAB_00463589:
        local_1c = 1;
    }
LAB_00463b55:
    local_1c = 1;
LAB_0046358c:
    uVar12 = tile->bit2 & 0xffdfffff;
LAB_00463603:
    *tile_bit2_ptr = uVar12;

    // THE CORNER MASKS: which corners of a land tile touch ocean (the
    // coastline gate), and the three 8-neighbour texture masks.
    coastal_mask[0] = 0;
    coastal_mask[1] = 0;
    coastal_mask[2] = 0;
    local_28 = 0;
    local_40 = 0;
    local_2c = 0;
    local_30 = 0;
    coastal_mask[3] = 0;
    local_20 = a6 & 0x80000000;
    if ((a6 & 0x80000000) == 0) {
        if (local_18 == 0) {
            uVar12 = 0;
            do {
                iVar5 = RadiusBaseX[uVar12] + a7;
                if ((MapIsFlat & 1) == 0) {
                    if (iVar5 < 0) {
                        iVar5 = MapLongitudeBounds + iVar5;
                    } else if (MapLongitudeBounds <= iVar5) {
                        iVar5 = iVar5 - MapLongitudeBounds;
                    }
                }
                iVar19 = RadiusBaseY[uVar12] + a8;
                if (iVar19 > -1 && iVar19 < MapLatitudeBounds &&
                    iVar5 > -1 && iVar5 < MapLongitudeBounds &&
                    (map_tiles()[*MapRowStride * iVar19 + (iVar5 >> 1)]
                         .climate &
                     0xe0) < 0x60) {
                    iVar5 = (int)uVar12 >> 1;
                    if ((uVar12 & 1) == 0) {
                        uVar17 = iVar5 + 1U & 3;
                        puVar8 = coastal_mask + (iVar5 - 2U & 3);
                        coastal_mask[uVar17] = coastal_mask[uVar17] | 4;
                        uVar17 = *puVar8 | 1;
                    } else {
                        uVar17 = iVar5 - 2U & 3;
                        puVar8 = coastal_mask + uVar17;
                        uVar17 = coastal_mask[uVar17] | 2;
                    }
                    *puVar8 = uVar17;
                }
                uVar12 = uVar12 + 1;
            } while ((int)uVar12 < 8);
            local_30 = (uint32_t)((local_34 & 0x80000004) == 4);
            if (local_84 != 0 || (local_24 & 0x20) != 0 || local_30 != 0) {
                iVar5 = 0;
                do {
                    uVar12 = iVar5 - 1U & 7;
                    iVar19 = RadiusBaseX[uVar12] + a7;
                    if ((MapIsFlat & 1) == 0) {
                        if (iVar19 < 0) {
                            iVar19 = iVar19 + MapLongitudeBounds;
                        } else if (MapLongitudeBounds <= iVar19) {
                            iVar19 = iVar19 - MapLongitudeBounds;
                        }
                    }
                    iVar6 = RadiusBaseY[uVar12] + a8;
                    if (iVar6 > -1 && iVar6 < MapLatitudeBounds &&
                        iVar19 > -1 && iVar19 < MapLongitudeBounds) {
                        pbVar18 = reinterpret_cast<uint8_t *>(
                            &map_tiles()[*MapRowStride * iVar6 +
                                         (iVar19 >> 1)]);
                        bVar11 = (uint8_t)iVar5;
                        if ((*(uint32_t *)(pbVar18 + 8) & 0x20) != 0 &&
                            0x3f < (*pbVar18 & 0xe0)) {
                            local_40 = local_40 | 1 << (bVar11 & 0x1f);
                        }
                        if (local_30 != 0 &&
                            (*(uint32_t *)(pbVar18 + 0xc) & 0x80000004) ==
                                4 &&
                            0x5f < (*pbVar18 & 0xe0)) {
                            local_2c |= 1u << (bVar11 & 0x1f);
                        }
                        if (local_84 != 0 &&
                            ((*pbVar18 & 0xe0) < 0x60 ||
                             (int)local_84 <=
                                 (int)(*pbVar18 >> 3 & 3))) {
                            local_28 |= 1u << (bVar11 & 0x1f);
                        }
                    }
                    iVar5 = iVar5 + 1;
                } while (iVar5 < 8);
            }
        } else if ((local_24 & 0x20) != 0) {
            iVar5 = 0;
            do {
                uVar12 = iVar5 - 1U & 7;
                iVar19 = RadiusBaseX[uVar12] + a7;
                if ((MapIsFlat & 1) == 0) {
                    if (iVar19 < 0) {
                        iVar19 = iVar19 + MapLongitudeBounds;
                    } else if (MapLongitudeBounds <= iVar19) {
                        iVar19 = iVar19 - MapLongitudeBounds;
                    }
                }
                iVar6 = RadiusBaseY[uVar12] + a8;
                if (iVar6 > -1 && iVar6 < MapLatitudeBounds &&
                    iVar19 > -1 && iVar19 < MapLongitudeBounds) {
                    pbVar18 = reinterpret_cast<uint8_t *>(
                        &map_tiles()[*MapRowStride * iVar6 + (iVar19 >> 1)]);
                    if ((*(uint32_t *)(pbVar18 + 8) & 0x20) != 0 &&
                        0x3f < (*pbVar18 & 0xe0)) {
                        local_40 = local_40 | 1 << ((uint8_t)iVar5 & 0x1f);
                    }
                }
                iVar5 = iVar5 + 1;
            } while (iVar5 < 8);
        }
    }
    a10 = a10 + local_78;
    a9 = a9 + local_78 * 2;

    // THE WIREFRAME EXIT: a far-out zoom or an unexplored tile draws only
    // the tile's diamond outline.
    if (0x15 < a5 ||
        (((GameDataStateStorage.fields_.game_rules_ & 0x100) == 0 && a4 > -1) &&
         ((PlayersData[a4].flags & 0x200) == 0 &&
          (tile->visibility & (uint8_t)(1u << (bVar10 & 0x1f))) == 0))) {
        if (local_20 == 0) {
            int step = 0;
            int tri = 0;
            do {
                for (int c = 0; c < 3; c++) {
                    land_vertices[tri * 3 + c].x =
                        LandCornerXStep[step] * local_78 + a9;
                    land_vertices[tri * 3 + c].y =
                        LandCornerYStep[step] * local_98 + a10;
                    if (window != 0) {
                        int delta = 0;
                        if (a8 < MapLatitudeBounds &&
                            a7 < MapLongitudeBounds) {
                            const int corner = LandCornerId[step];
                            window->get_alt(a7, a8, corner);
                            if (window->owned_ == 0) {
                                delta = 0;
                            } else {
                                delta = reinterpret_cast<char *>(
                                            window->owned_)
                                    [((MapLongitudeBounds >> 1) * a8 +
                                      local_14) *
                                         0xc +
                                     4 + corner];
                            }
                        }
                        land_vertices[tri * 3 + c].y -= delta;
                    }
                    step = step + 1;
                }
                MapCornerScratch[tri].x = land_vertices[tri * 3 + 1].x;
                MapCornerScratch[tri].y = land_vertices[tri * 3 + 1].y;
                tri = tri + 1;
            } while (tri < 4);
            for (uVar12 = 1; uVar12 <= 4; uVar12++) {
                buffer->line(
                    MapCornerScratch[(uVar12 - 1)].x,
                    MapCornerScratch[(uVar12 - 1)].y,
                    MapCornerScratch[(uVar12 & 3)].x,
                    MapCornerScratch[(uVar12 & 3)].y, 0xc2);
            }
            if (ConsoleGlobal.cursor_history_x_
                        [ConsoleGlobal.cursor_history_index_] == a7 &&
                ConsoleGlobal.cursor_history_y_
                        [ConsoleGlobal.cursor_history_index_] == a8) {
                for (iVar5 = 0; iVar5 < 4; iVar5++) {
                    MapCornerCopy[iVar5] = MapCornerScratch[iVar5];
                }
            }
        }
        goto LAB_00466c32;
    }

    // Four triangle fans, three vertices apiece. The original uses byte
    // offsets (463E99-463F8B); these arrays keep vertex/UV units explicit.
    local_bc = 0;
    local_c4 = local_rand.get(0, 2);
    skip_terrain = a6 & 0x10000000;
    if (skip_terrain != 0) {
        goto LAB_00465605;
    }
    if ((a6 & 0x10000000) == 0) {
        for (int tri = 0; tri < 4; ++tri) {
            for (int corner = 0; corner < 3; ++corner) {
                const int vertex = tri * 3 + corner;
                land_vertices[vertex].x =
                    LandCornerXStep[vertex] * local_78 + a9;
                land_vertices[vertex].y =
                    LandCornerYStep[vertex] * local_98 + a10;
                int uv = vertex;
                if (local_18 == 0 && (local_24 & 0x8000) == 0 &&
                    local_84 != 0 && local_20 == 0) {
                    uv = corner +
                        ((LandMoistureTable[local_28][1] + tri) & 3) * 3;
                }
                // Both paths use 686090, not the overlay's shifted 686098.
                land_uv[vertex].x = LandCornerUVs[uv].x;
                land_uv[vertex].y = LandCornerUVs[uv].y;
                if (local_38 != 0) {
                    rocky_uv[vertex].x = LandCornerUVs[vertex].x;
                    rocky_uv[vertex].y = LandCornerUVs[vertex].y;
                }
                if (2 < (int)local_8c && local_84 != 0) {
                    local_bc = LandMoistureTable[local_28][0];
                }
            }
        }
        // 463F9F-46401E initializes four quads, not eight vertices.
        for (int corner = 0; corner < 4; ++corner) {
            for (int sub = 0; sub < 4; ++sub) {
                const int vertex = corner * 4 + sub;
                const int direction = (corner - 2 + sub) & 3;
                edge_vertices[vertex].x =
                    CornerOffsetX[corner][direction] * local_78 + a9;
                edge_vertices[vertex].y =
                    CornerOffsetY[corner][direction] * local_98 + a10;
                edge_uv[vertex].x = EdgeMidUVs[vertex].x;
                edge_uv[vertex].y = EdgeMidUVs[vertex].y;
            }
        }
    }
    local_80 = 0;
    local_c0 = a6 & 0x10000;
    if (local_c0 != 0 &&
        (((PlayersData[ConsoleGlobal.faction_id_].flags & 0x200) != 0 ||
          (tile->visibility &
           (uint8_t)(1 << ((uint8_t)ConsoleGlobal.faction_id_ & 0x1f))) !=
              0) ||
         (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) &&
        local_20 != 0) {
        local_80 = 1;
        territory_faction = whose_territory(ConsoleGlobal.faction_id_, a7, a8, 0, 0);
    }

    // Each triangle publishes its middle vertex and light as one corner
    // of the shared diamond. The three lights form one real array.
    {
        for (int tri = 0; tri < 4; ++tri) {
            for (int corner = 0; corner < 3; ++corner) {
                const int vertex = tri * 3 + corner;
                if ((int)local_8c < 3) {
                    triangle_light[corner] = tri;
                } else if (window != 0) {
                    land_vertices[vertex].y -=
                        window->get_alt(a7, a8, LandCornerId[vertex]);
                    triangle_light[corner] = local_1c == 0
                        ? UnworkedCornerLight
                        : window->get_point_light(a7, a8, LandCornerId[vertex]);
                } else {
                    triangle_light[corner] = 0;
                }
            }
            MapCornerScratch[tri].x = land_vertices[tri * 3 + 1].x;
            MapCornerScratch[tri].y = land_vertices[tri * 3 + 1].y;
            local_b8[tri] = triangle_light[1];
            if (local_18 == 0 && (local_80 == 0 || (int)territory_faction < 1) &&
                local_20 == 0) {
                const int shade =
                    (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41;
                Vert *const verts = land_vertices + tri * 3;
                Vert *const uvs = land_uv + tri * 3;
                if (((local_24 & 0x8000) == 0) || ((a6 & 1) == 0)) {
                    if ((local_34 & 0x40) != 0) {
                        g_DUNE_LAND_TEXTURE.draw(
                            buffer, verts, triangle_light, 3,
                            LandCornerUVs + tri * 3, shade);
                    } else if (local_84 == 2) {
                        g_RAINY_LAND_TEXTURES[local_bc].draw(
                            buffer, verts, triangle_light, 3, uvs, shade);
                    } else if (local_84 == 1) {
                        g_MOIST_LAND_TEXTURES[local_bc].draw(
                            buffer, verts, triangle_light, 3, uvs, shade);
                    } else if (local_84 == 0) {
                        g_FLAT_ARID_LAND_TEXTURE.draw(
                            buffer, verts, triangle_light, 3, uvs, shade);
                    }
                } else {
                    g_FARM_TEXTURES[local_84 + 3 * local_c4].draw(
                        buffer, verts, triangle_light, 3, uvs, shade);
                }
                if (local_38 != 0) {
                    g_ROCKY_TEXTURES[local_c4 + 2 * local_38 - 2].draw_trans_3(
                        buffer, verts, triangle_light, 3, rocky_uv + tri * 3, shade);
                }
            }
        }
    }

    // THE SOLID-FILL EXIT: editor solid mode, known enemy
    // territory, or the no-terrain flag stops at one flat polygon.
    if (local_11c != 0) {
        int fill;
        if (local_18 != 0) {
            fill = 0xa1;
        } else if (territory_faction == 0xffffffff) {
            fill = 0x11;
        } else {
LAB_0046441f:
            fill = InfoFactionTextColors[territory_faction];
        }
        buffer->poly(MapCornerScratch, 4, fill);
        TextureLightFloor = 0xffffff00;
        TextureLightCeiling = 0x100;
        return 1;
    }
    if (local_18 == 0) {
        if (local_80 != 0 && 0 < (int)territory_faction) {
            goto LAB_0046441f;
        }
        if (local_20 != 0) {
            int fill;
            if ((a6 & 0x10) == 0) {
                fill = (0xe - (int)local_8c) * 2;
            } else {
                fill = ((tile->climate & 0x18) >> 3) * 2 + 0x95;
            }
            buffer->poly(MapCornerScratch, 4,
                         fill);
            TextureLightFloor = 0xffffff00;
            TextureLightCeiling = 0x100;
            return 1;
        }
    }

    // Rotated edge quads join each fan's outer vertices and neighboring
    // edge midpoints (46447D-464544).
    {
        for (int fan_index = 0; fan_index < 4; ++fan_index) {
            const int next = (fan_index + 1) & 3;
            Vert *const edge = edge_vertices + next * 4;
            const Vert *const fan = land_vertices + fan_index * 3;
            const Vert *const neighbor = land_vertices + next * 3;
            edge[0] = fan[0];
            edge[1].x = (fan[1].x + fan[2].x) >> 1;
            edge[1].y = (fan[1].y + fan[2].y) >> 1;
            edge[2] = fan[2];
            edge[3].x = (neighbor[2].x + neighbor[1].x) >> 1;
            edge[3].y = (neighbor[2].y + neighbor[1].y) >> 1;
        }
    }

    // THE OCEAN PASS: per corner, four sub-vertices at the tiles that meet
    // there, shaded through alt_get_ocean_detail (sites 4/5/6) and then
    // the ocean wireframe fill. The image's per-corner shade run lives at
    // 0x7F6644 and its vertices at 0x7D3AF0.
    if ((a6 & 0x2000000) == 0) {
        int corner = 0;  // the a5 slot again
        int uv_off = 0;
        do {
            if (local_18 != 0 || coastal_mask[corner] != 0) {
                int *const shades = OceanCornerShades + corner * 4;
                Vert *const uvs = ocean_uv + corner * 4;
                for (int sub = 0; sub < 4; sub++) {
                    shades[sub] = 0;
                    const int direction = (corner - 2 + sub) & 3;
                    Vert *const vert =
                        OceanCornerVerts + corner * 4 + sub;
                    vert->x = CornerOffsetX[corner][direction] * local_78 + a9;
                    vert->y = CornerOffsetY[corner][direction] * local_98 + a10;
                    uvs[sub].x =
                        OceanCornerUVs[corner * 4 + sub].x;
                    uvs[sub].y =
                        OceanCornerUVs[corner * 4 + sub].y;
                    shades[sub] =
                        OceanDetailShade[alt_get_ocean_detail(
                            a7, a8, corner, sub)];
                }
                if (local_18 != 0 && local_20 == 0) {
                    if (0xf < shades[0] || 0xf < shades[1] ||
                        0xf < shades[2] || 0xf < shades[3]) {
                        int ocean_zvals[4];
                        for (int sub = 0; sub < 4; sub++) {
                            ocean_zvals[sub] = shades[sub] - 0x10;
                        }
                        const int shade =
                            (-(uint32_t)(local_1c != 0) & 0xffffffbe) +
                            0x41;
                        if (local_1c != 0) {
                            g_OCEAN_TEXTURES[1].draw(
                                buffer, OceanCornerVerts + corner * 4,
                                ocean_zvals, 4, uvs);
                        } else {
                            g_OCEAN_TEXTURES[1].draw(
                                buffer, OceanCornerVerts + corner * 4,
                                ocean_zvals, 4, uvs, shade);
                        }
                    }
                    g_OCEAN_TEXTURES[0].draw2(
                        buffer, OceanCornerVerts + corner * 4, shades, 4,
                        uvs, 0x10,
                        (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
                }
            }
            corner = corner + 1;
            uv_off = uv_off + 8;
        } while (corner < 4);
    }
    if (local_18 == 0 || local_20 == 0) {
        goto landmark_chain;
    }
    buffer->poly(MapCornerScratch, 4, 0xbe);
    TextureLightFloor = 0xffffff00;
    TextureLightCeiling = 0x100;
    return 1;

landmark_chain:;
    // THE LANDMARK OVERLAYS (sites 7-10): one quad per family, receiver
    // index from the sequence-indexed table, vertices from the corner
    // scratch the per-triangle loop filled.
    iVar5 = local_94;
    if (local_18 == 0 || local_20 == 0) {
        if (local_70 == 0 && (local_34 & 2) != 0 && -1 < local_94 &&
            local_94 < static_cast<int>(sizeof(MountPlanetTable) / sizeof(MountPlanetTable[0])) &&
            MountPlanetTable[local_94].texture != -1) {
            uVar12 = (uint32_t)MountPlanetTable[local_94].rotation;
            {
                Vert quad[4];
                int quad_z[4];
                const uint32_t k0 = uVar12 & 3;
                const uint32_t k1 = (uVar12 + 1) & 3;
                const uint32_t k2 = (uVar12 - 2) & 3;
                const uint32_t k3 = (uVar12 - 1) & 3;
                quad[0].x = MapCornerScratch[k0].x;
                quad[0].y = MapCornerScratch[k0].y;
                quad[1].x = MapCornerScratch[k1].x;
                quad[1].y = MapCornerScratch[k1].y;
                quad[2].x = MapCornerScratch[k2].x;
                quad[2].y = MapCornerScratch[k2].y;
                quad[3].x = MapCornerScratch[k3].x;
                quad[3].y = MapCornerScratch[k3].y;
                quad_z[0] = local_b8[k0];
                quad_z[1] = local_b8[k1];
                quad_z[2] = local_b8[k2];
                quad_z[3] = local_b8[k3];
            overlay_uv[0].x = 0;
            overlay_uv[0].y = 0;
            overlay_uv[1].x = 0x37;
            overlay_uv[1].y = 0;
            overlay_uv[2].x = 0x37;
            overlay_uv[2].y = 0x37;
            overlay_uv[3].x = 0;
            overlay_uv[3].y = 0x37;
            if (local_94 == 0) {
                g_MOUNT_PLANET_TEXTURES[MountPlanetTable[local_94].texture]
                    .draw_color_key(
                        buffer, quad, 4,
                        overlay_uv);
            } else {
                g_MOUNT_PLANET_TEXTURES[MountPlanetTable[local_94].texture]
                    .draw_trans(
                        buffer, quad, quad_z, 4,
                        overlay_uv);
            }
            }
        }
        if ((local_34 & 0x100) != 0 && local_70 == 0 && -1 < iVar5 &&
            iVar5 < static_cast<int>(sizeof(SunnyMesaTable) / sizeof(SunnyMesaTable[0])) &&
            SunnyMesaTable[iVar5].texture != -1) {
            uVar12 = (uint32_t)SunnyMesaTable[iVar5].rotation;
            {
                Vert quad[4];
                int quad_z[4];
                const uint32_t k0 = uVar12 & 3;
                const uint32_t k1 = (uVar12 + 1) & 3;
                const uint32_t k2 = (uVar12 - 2) & 3;
                const uint32_t k3 = (uVar12 - 1) & 3;
                quad[0].x = MapCornerScratch[k0].x;
                quad[0].y = MapCornerScratch[k0].y;
                quad[1].x = MapCornerScratch[k1].x;
                quad[1].y = MapCornerScratch[k1].y;
                quad[2].x = MapCornerScratch[k2].x;
                quad[2].y = MapCornerScratch[k2].y;
                quad[3].x = MapCornerScratch[k3].x;
                quad[3].y = MapCornerScratch[k3].y;
                quad_z[0] = local_b8[k0];
                quad_z[1] = local_b8[k1];
                quad_z[2] = local_b8[k2];
                quad_z[3] = local_b8[k3];
            overlay_uv[2].x = 0x37;
            overlay_uv[1].x = 0;
            overlay_uv[1].y = 0;
            overlay_uv[2].y = 0;
            overlay_uv[0].x = 0;
            overlay_uv[3].x = 0x37;
            overlay_uv[3].y = 0x37;
            overlay_uv[0].y = 0x37;
            g_SUNNY_MESA_TEXTURES[SunnyMesaTable[iVar5].texture]
                .draw_trans(buffer, quad, quad_z, 4,
                            overlay_uv);
            }
        }
        if ((local_34 & 1) != 0 && local_70 == 0 && -1 < iVar5 &&
            iVar5 < static_cast<int>(sizeof(GarlandCraterTable) / sizeof(GarlandCraterTable[0])) &&
            GarlandCraterTable[iVar5].texture != -1) {
            uVar12 = (uint32_t)GarlandCraterTable[iVar5].rotation;
            {
                Vert quad[4];
                int quad_z[4];
                const uint32_t k0 = uVar12 & 3;
                const uint32_t k1 = (uVar12 + 1) & 3;
                const uint32_t k2 = (uVar12 - 2) & 3;
                const uint32_t k3 = (uVar12 - 1) & 3;
                quad[0].x = MapCornerScratch[k0].x;
                quad[0].y = MapCornerScratch[k0].y;
                quad[1].x = MapCornerScratch[k1].x;
                quad[1].y = MapCornerScratch[k1].y;
                quad[2].x = MapCornerScratch[k2].x;
                quad[2].y = MapCornerScratch[k2].y;
                quad[3].x = MapCornerScratch[k3].x;
                quad[3].y = MapCornerScratch[k3].y;
                quad_z[0] = local_b8[k0];
                quad_z[1] = local_b8[k1];
                quad_z[2] = local_b8[k2];
                quad_z[3] = local_b8[k3];
            overlay_uv[2].x = 0x37;
            overlay_uv[1].x = 0;
            overlay_uv[1].y = 0;
            overlay_uv[2].y = 0;
            overlay_uv[0].x = 0;
            overlay_uv[3].x = 0x37;
            overlay_uv[3].y = 0x37;
            overlay_uv[0].y = 0x37;
            g_GARLAND_CRATER_TEXTURES[GarlandCraterTable[iVar5].texture]
                .draw_trans(buffer, quad, quad_z, 4,
                            overlay_uv);
            }
        }

        // THE VIEW BORDER LINES: with the show-grid preference or the
        // overlay flag, the tile outline is drawn 0x25/0xbe between this
        // tile's corners and its west/north neighbours' (site: Buffer::line
        // at 0x464CB3).
        if ((((GameDataStateStorage.fields_.game_preferences_ & 0x100) != 0 && local_20 == 0) ||
             ((a6 & 0x4000000) != 0)) &&
            ((0x5f < (tile->climate & 0xe0)) || local_20 != 0 ||
             ((GameDataStateStorage.fields_.game_more_preferences_ & 0x800000) != 0))) {
            iVar5 = a7 + -1;
            if ((MapIsFlat & 1) == 0) {
                if (iVar5 < 0) {
                    iVar5 = iVar5 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar5) {
                    iVar5 = iVar5 - MapLongitudeBounds;
                }
            }
            uVar12 = 1;
            do {
                iVar19 = a8 + -1;
                if (iVar19 > -1 && iVar19 < MapLatitudeBounds &&
                    iVar5 > -1 && iVar5 < MapLongitudeBounds) {
                    bVar11 = map_tiles()[*MapRowStride * iVar19 +
                                         (iVar5 >> 1)]
                                 .climate &
                             0xe0;
                    if (bVar11 < 0x60) {
                        if (local_20 != 0 ||
                            ((GameDataStateStorage.fields_.game_more_preferences_ & 0x800000) != 0)) {
                            if (0x5f < bVar11) {
                                goto border_bright;
                            }
                            goto border_dim;
                        }
                    } else {
border_bright:
                        uVar15 = 0x25;
                        if ((tile->climate & 0xe0) < 0x60) {
border_dim:
                            uVar15 = 0xbe;
                        }
                        buffer->line(
                            MapCornerScratch[(uVar12 - 1)].x,
                            MapCornerScratch[(uVar12 - 1)].y,
                            MapCornerScratch[(uVar12 & 3)].x,
                            MapCornerScratch[(uVar12 & 3)].y,
                            uVar15);
                    }
                }
                iVar5 = iVar5 + 2;
                if ((MapIsFlat & 1) == 0) {
                    if (iVar5 < 0) {
                        iVar5 = iVar5 + MapLongitudeBounds;
                    } else if (MapLongitudeBounds <= iVar5) {
                        iVar5 = iVar5 - MapLongitudeBounds;
                    }
                }
                uVar12 = uVar12 + 1;
            } while (uVar12 <= 2);
        }
        if (ConsoleGlobal.cursor_history_x_
                    [ConsoleGlobal.cursor_history_index_] == a7 &&
            ConsoleGlobal.cursor_history_y_
                    [ConsoleGlobal.cursor_history_index_] == a8) {
            for (iVar5 = 0; iVar5 < 4; iVar5++) {
                MapCornerCopy[iVar5] = MapCornerScratch[iVar5];
            }
        }
        a9 = a9 + local_78 * -2;
        a10 = a10 - (local_44 + local_78);
        if (MapWinCaptionScale < 1) {
            SpriteScaleDenominator = 0x10;
            SpriteScaleNumerator = a3 + 0x10;
        } else {
            SpriteScaleNumerator = MapWinCaptionScale;
            SpriteScaleDenominator = MapWinCaptionDivisor;
        }

        // THE LANDMARK RESOURCE SPRITE (site 21).
        if ((a6 & 0x200) != 0 && local_70 == 0) {
            uVar12 = tile->bit2;
            if ((uVar12 & 0x80000008) == 8) {
                g_TER1_LANDMARK_RESOURCE_SPRITES[4].draw(
                    buffer, sprite_transparent(g_TER1_LANDMARK_RESOURCE_SPRITES[4]),
                    a9, a10);
            } else if ((uVar12 & 0x80000400) == 0x400) {
                g_TER1_LANDMARK_RESOURCE_SPRITES[5].draw(
                    buffer, sprite_transparent(g_TER1_LANDMARK_RESOURCE_SPRITES[5]),
                    a9, a10);
            }
        }

        // THE JUNGLE OVERLAY (site 11), gated by base radius, unworked and
        // unexplored-clear. The corner mask loop lands in the dead a5
        // parameter slot; the receiver indexes with the EARLIER 8-neighbour
        // base mask (see the header note).
        if (local_30 != 0 && local_18 == 0 && local_70 == 0) {
            a5 = 0;  // the image's dead-slot mask
            for (int k = 0; k < 4; k++) {
                int nx = RiverStepX[k] + a7;
                if ((MapIsFlat & 1) == 0) {
                    if (nx < 0) {
                        nx += MapLongitudeBounds;
                    } else if (MapLongitudeBounds <= nx) {
                        nx -= MapLongitudeBounds;
                    }
                }
                const int ny = RiverStepY[k] + a8;
                if (ny > -1 && ny < MapLatitudeBounds && nx > -1 &&
                    nx < MapLongitudeBounds) {
                    Map *const ntile =
                        &map_tiles()[*MapRowStride * ny + (nx >> 1)];
                    if ((ntile->bit2 & 0x80000004) == 4 &&
                        0x5f < (ntile->climate & 0xe0)) {
                        a5 |= 1 << k;
                    }
                }
            }
            TextureRasterKey = 9;
            uVar12 =
                (uint32_t)(uint8_t)LandMoistureTable[local_2c][1];
            {
                int idx = 0;
                for (int e = 0; e < 4; e++) {
                    const uint32_t uVar17 = uVar12 & 3;
                    uVar12 = uVar12 + 1;
                    overlay_uv[e].x = LandCornerUVs[uVar17 * 3 + 1].x;
                    overlay_uv[e].y = LandCornerUVs[uVar17 * 3 + 1].y;
                    idx += 2;
                }
            }
            g_JUNGLE_LAND_TEXTURES[LandMoistureTable[local_2c][0]]
                .draw_color_key(buffer,
                                MapCornerScratch,
                                4, overlay_uv);
        }

        // THE FOREST OVERLAY (site 12): receiver = the four-corner fungus
        // mask the loop above this in the image collects afresh here.
        if ((local_24 & 0x200000) != 0 && 2 < (int)local_8c &&
            (a6 & 1) != 0 && local_70 == 0) {
            uVar12 = 0;  // esi in the image
            for (int k = 0; k < 4; k++) {
                int nx = RiverStepX[k] + a7;
                if ((MapIsFlat & 1) == 0) {
                    if (nx < 0) {
                        nx += MapLongitudeBounds;
                    } else if (MapLongitudeBounds <= nx) {
                        nx -= MapLongitudeBounds;
                    }
                }
                const int ny = RiverStepY[k] + a8;
                if (ny > -1 && ny < MapLatitudeBounds && nx > -1 &&
                    nx < MapLongitudeBounds) {
                    Map *const ntile =
                        &map_tiles()[*MapRowStride * ny + (nx >> 1)];
                    if ((ntile->bit & 0x200000) != 0) {
                        uVar12 |= 1u << k;
                    }
                }
            }
            overlay_uv[1].x = 0x37;
            overlay_uv[2].x = 0x37;
            overlay_uv[2].y = 0x37;
            overlay_uv[3].y = 0x37;
            overlay_uv[0].x = 0;
            overlay_uv[0].y = 0;
            overlay_uv[1].y = 0;
            overlay_uv[3].x = 0;
            TextureRasterKey = 9;
            g_FOREST_TEXTURES[uVar12].draw_trans(
                buffer, MapCornerScratch,
                local_b8, 4, overlay_uv);
        }

        // THE FUNGUS OVERLAY (site 13): receiver row from the 8-neighbour
        // land-with-fungus mask, +15 for water. The shade uses worked state.
        if ((local_24 & 0x20) != 0 && 1 < (int)local_8c && local_70 == 0 &&
            -10 < a3) {
            TextureRasterKey = 9;
            uVar12 =
                (uint32_t)(uint8_t)LandMoistureTable[local_40][1];
            {
                int idx = 0;
                for (int e = 0; e < 4; e++) {
                    const uint32_t uVar17 = uVar12 & 3;
                    uVar12 = uVar12 + 1;
                    overlay_uv[e].x = LandCornerUVs[uVar17 * 3 + 1].x;
                    overlay_uv[e].y = LandCornerUVs[uVar17 * 3 + 1].y;
                    idx += 2;
                }
            }
            g_FUNGUS_TEXTURES[LandMoistureTable[local_40][0] +
                              15 * (local_18 != 0)]
                .draw_color_key(
                    buffer, MapCornerScratch,
                    4, overlay_uv,
                    (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
        }

        // THE TERRITORY BORDER (site 14): the per-corner faction bytes the
        // 8-neighbour walk collected, remapped through the shared table.
        if (local_c0 != 0 &&
            (((PlayersData[ConsoleGlobal.faction_id_].flags & 0x200) !=
                  0 ||
              (tile->visibility &
               (uint8_t)(1 << ((uint8_t)ConsoleGlobal.faction_id_ &
                               0x1f))) != 0) ||
             (GameDataStateStorage.fields_.game_state_ & 0x80) != 0) &&
            local_20 == 0) {
            territory_faction = whose_territory(ConsoleGlobal.faction_id_, a7,
                                          a8, 0, 0);
            if (-1 < (int)territory_faction) {
                territory_mask[1] = 0;
                territory_mask[2] = 0;
                territory_mask[3] = 0;
                territory_mask[4] = 0;
                territory_mask[0] = 0;
                territory_mask[5] = 0;
                territory_mask[6] = 0;
                territory_mask[7] = 0;
                territory_mask[8] = 0;
                local_44 = 0;
                territory_mask[9] = 0;
                local_c0 = (uint32_t)tile->contour;
                for (int k = 0; k < 8; k++) {
                    iVar5 = RadiusBaseX[k] + a7;
                    if ((MapIsFlat & 1) == 0) {
                        if (iVar5 < 0) {
                            iVar5 = MapLongitudeBounds + iVar5;
                        } else if (MapLongitudeBounds <= iVar5) {
                            iVar5 = iVar5 - MapLongitudeBounds;
                        }
                    }
                    iVar19 = RadiusBaseY[k] + a8;
                    if (iVar19 > -1 && iVar19 < MapLatitudeBounds &&
                        iVar5 > -1 && iVar5 < MapLongitudeBounds &&
                        (map_tiles()[*MapRowStride * iVar19 + (iVar5 >> 1)]
                             .climate &
                         0xe0) < 0x60 ==
                            (tile->climate & 0xe0) < 0x60 &&
                        map_tiles()[*MapRowStride * iVar19 + (iVar5 >> 1)]
                                .contour == local_c0) {
                        uVar12 = whose_territory(
                            ConsoleGlobal.faction_id_, iVar5, iVar19, 0,
                            0);
                        if (territory_faction != uVar12) {
                            local_44 = 1;
                            cVar4 = (char)territory_faction;
                            if (((uint32_t)k & 1) == 0) {
                                if (k == 0) {
                                    territory_mask[8] = cVar4 + 1;
                                    territory_mask[2] = cVar4 + 1;
                                    territory_mask[1] = cVar4 + 1;
                                    goto LAB_004652ed;
                                }
                                territory_mask[k] = cVar4 + 1;
                                territory_mask[k + 2] = cVar4 + 1;
                            }
                            territory_mask[k + 1] = cVar4 + 1;
                        }
LAB_004652ed:;
                    }
                }
                if (local_44 != 0) {
                    overlay_uv[1].x = 0x3b;
                    overlay_uv[2].x = 0x3b;
                    overlay_uv[2].y = 0x3b;
                    overlay_uv[3].y = 0x3b;
                    overlay_uv[0].x = 0;
                    overlay_uv[0].y = 0;
                    overlay_uv[1].y = 0;
                    overlay_uv[3].x = 0;
                    g_RAINFALL_SINGLE_TILE_TEXTURE.draw_mask_dest(
                        buffer,
                        MapCornerScratch,
                        territory_mask,
                        reinterpret_cast<uint8_t *>(TerritoryColorTables),
                        4, overlay_uv);
                }
            }
        }

        // THE COAST OVERLAY (site 15): the 53-entry mask table per corner
        // code, blitted against the rainfall single-tile band.
        if (-8 < a3 && local_18 == 0) {
            for (int corner = 0; corner < 4; corner++) {
                uVar12 = (uint32_t)coastal_mask[corner];
                if (uVar12 != 0) {
                    iVar19 = 0;
                    do {
                        bVar11 = CoastMaskTable[iVar19][uVar12];
                        coast_remap[iVar19] = bVar11;
                        if (1 < bVar11) {
                            coast_remap[iVar19] =
                                TerrainCoastPalette[bVar11 + 2];
                        }
                        iVar19 = iVar19 + 1;
                    } while (iVar19 < 0x35);
                    if ((uVar12 == 7) && ((a8 - a7 & 2) != 0)) {
                        coast_remap[5] = 0;
                        coast_remap[0xe] = 0;
                        coast_remap[0xf] = 0;
                        coast_remap[0x10] = 0;
                        coast_remap[0x11] = 0;
                        // All three runs read the same four published colors
                        // backward, not separate fixed-address storage.
                        for (int run = 0; run < 3; ++run) {
                            for (int shade = 0; shade < 4; ++shade) {
                                coast_remap[0x12 + run * 4 + shade] =
                                    TerrainCoastPalette[7 - shade];
                            }
                        }
                    }
                    // The image copies the mid-UV run for this corner
                    // and passes the ocean shade run as zvals.
                    for (int e = 0; e < 4; e++) {
                        ocean_uv[corner * 4 + e] = edge_uv[corner * 4 + e];
                    }
                    g_OCEAN_TEXTURES[0].draw_coast(
                        buffer,
                        edge_vertices + corner * 4,
                        OceanCornerShades + corner * 4,
                        &g_RAINFALL_SINGLE_TILE_TEXTURES[0], coast_remap,
                        4, ocean_uv + corner * 4,
                        (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
                }
            }
        }

        // THE RIVER OVERLAYS (sites 16/17): the corner river mask the
        // diagonal walk collects selects the receiver row.
        if ((a6 & 0x400) != 0 &&
            (tile->bit & 0x80) != 0 && local_18 == 0) {
            uVar12 = 0;  // ebx in the image
            for (int k = 0; k < 4; k++) {
                int nx = RiverStepX[k] + a7;
                if ((MapIsFlat & 1) == 0) {
                    if (nx < 0) {
                        nx += MapLongitudeBounds;
                    } else if (MapLongitudeBounds <= nx) {
                        nx -= MapLongitudeBounds;
                    }
                }
                const int ny = RiverStepY[k] + a8;
                if (ny > -1 && ny < MapLatitudeBounds && nx > -1 &&
                    nx < MapLongitudeBounds &&
                    (map_tiles()[*MapRowStride * ny + (nx >> 1)].bit &
                     0x80) != 0) {
                    uVar12 |= 1u << k;
                }
            }
            overlay_uv[0].x = 0;
            overlay_uv[1].x = 0x37;
            overlay_uv[2].x = 0x37;
            overlay_uv[2].y = 0x37;
            overlay_uv[3].y = 0x37;
            overlay_uv[0].y = 0;
            overlay_uv[1].y = 0;
            overlay_uv[3].x = 0;
            TextureRasterKey = 9;
            g_RIVER_TEXTURES[uVar12].draw_trans_3(
                buffer, MapCornerScratch,
                local_b8, 4, overlay_uv,
                (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
            if ((tile->bit & 0x200) != 0) {
                g_RIVER_TEXTURES[0].draw_trans_3(
                    buffer, MapCornerScratch,
                    local_b8, 4, overlay_uv,
                    (-(uint32_t)(local_1c != 0) & 0xffffffbe) + 0x41);
            }
        }
        goto LAB_00465605;
    }

LAB_00465605:;
    // THE SENSOR SWEEP: the distance class between the map focus tile and
    // this tile (and each corner) decides whether the tile outline joins
    // the sweep display (Buffer::line at 0x465A3B).
    focus_y = MapFocusY;
    if (MapWinCaptionScale < 1) {
        SpriteScaleDenominator = 0x10;
        SpriteScaleNumerator = a3 + 0x10;
    } else {
        SpriteScaleNumerator = MapWinCaptionScale;
        SpriteScaleDenominator = MapWinCaptionDivisor;
    }
    if ((local_20 != 0 && skip_terrain == 0) ||
        (a4 > -1 && (PlayersData[a4].flags & 0x200) == 0 &&
         (tile->visibility & (uint8_t)(1u << (bVar10 & 0x1f))) == 0)) {
        goto LAB_00466c32;
    }
    if ((GameDataStateStorage.fields_.game_preferences_ & 0x200) != 0 && local_20 == 0) {
        uVar12 = tile->val3 & 7;
        if (a4 > -1 && (int)uVar12 != a4 &&
            (PlayersData[a4].diplo_treaties[uVar12] & 8) == 0) {
            uVar12 = 0;
        }
        iVar5 = abs(a7 - MapFocusX);
        if ((MapIsFlat & 1) == 0 && (int)MapLongitude < iVar5) {
            iVar5 = MapLongitudeBounds - iVar5;
        }
        uVar15 = abs(a8 - focus_y);
        iVar19 = abs(iVar5);
        iVar6 = abs(uVar15);
        iVar5 = iVar19 <= iVar6 ? iVar6 : iVar19;
        iVar7 = iVar6 <= iVar19 ? iVar6 : iVar19;
        if ((iVar5 - (((iVar19 + iVar6 >> 1) - iVar7) + 1 >> 1) < 3) +
                uVar12 ==
            0) {
            iVar5 = abs(a7 - MapFocusX);
            if ((MapIsFlat & 1) == 0 && (int)MapLongitude < iVar5) {
                iVar5 = MapLongitudeBounds - iVar5;
            }
            uVar15 = abs(a8 - focus_y);
            iVar19 = abs(iVar5);
            iVar6 = abs(uVar15);
            iVar5 = iVar19 <= iVar6 ? iVar6 : iVar19;
            iVar7 = iVar6 <= iVar19 ? iVar6 : iVar19;
            local_44 = (uint32_t)((-1 < (int)(((iVar5 -
                                                  (((iVar19 + iVar6 >> 1) -
                                                    iVar7) +
                                                   1 >> 1)) < 3) +
                                                 uVar12)) -
                                  1);
        } else {
            local_44 = 1;
        }
        local_30b = 1;
        local_2c = 0;
        do {
            iVar5 = RiverStepX[local_30b - 1] + a7;
            if ((MapIsFlat & 1) == 0) {
                if (iVar5 < 0) {
                    iVar5 = iVar5 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar5) {
                    iVar5 = iVar5 - MapLongitudeBounds;
                }
            }
            local_34b = RiverStepY[local_30b - 1] + a8;
            if ((int)local_34b > -1 && (int)local_34b < MapLatitudeBounds &&
                iVar5 > -1 && iVar5 < MapLongitudeBounds) {
                uVar17 = (uint32_t)(
                    map_tiles()[*MapRowStride * (int)local_34b + (iVar5 >> 1)]
                        .val3 &
                    7);
                if (a4 > -1 && uVar17 != (uint32_t)a4 &&
                    (PlayersData[a4].diplo_treaties[uVar17] & 8) == 0) {
                    uVar17 = 0;
                }
                focus_y = MapFocusY;
                iVar19 = abs(iVar5 - MapFocusX);
                if ((MapIsFlat & 1) == 0 && (int)MapLongitude < iVar19) {
                    iVar19 = MapLongitudeBounds - iVar19;
                }
                local_48b = (uint32_t)abs((int)local_34b - focus_y);
                iVar6 = abs(iVar19);
                iVar7 = abs((int)local_48b);
                iVar19 = iVar6 <= iVar7 ? iVar7 : iVar6;
                iVar13 = iVar7 <= iVar6 ? iVar7 : iVar6;
                if ((uint32_t)((iVar19 -
                                (((iVar6 + iVar7 >> 1) - iVar13) +
                                 1 >> 1) < 3) +
                               uVar17) == 0) {
                    iVar5 = abs(iVar5 - MapFocusX);
                    if ((MapIsFlat & 1) == 0 && (int)MapLongitude < iVar5) {
                        iVar5 = MapLongitudeBounds - iVar5;
                    }
                    uVar15 = abs((int)local_34b - focus_y);
                    iVar19 = abs(iVar5);
                    iVar6 = abs(uVar15);
                    iVar5 = iVar19 <= iVar6 ? iVar6 : iVar19;
                    iVar7 = iVar6 <= iVar19 ? iVar6 : iVar19;
                    uVar12 = (uint32_t)((-1 < (int)(((iVar5 -
                                                       (((iVar19 + iVar6 >>
                                                           1) -
                                                          iVar7) +
                                                         1 >> 1)) <
                                                      3) +
                                                     uVar17)) -
                                        1);
                } else {
                    uVar12 = 1;
                }
                if (uVar12 != local_44) {
                    uVar17 = local_30b + 1 & 3;
                    buffer->line(
                        MapCornerScratch[(local_30b & 3)].x,
                        MapCornerScratch[(local_30b & 3)].y,
                        MapCornerScratch[uVar17].x,
                        MapCornerScratch[uVar17].y, 0xf9);
                }
            }
            local_30b = local_30b + 1;
        } while (local_30b < 5);
    }

    // THE KELP FARM SPRITE (site 22).
    if ((a6 & 1) != 0 && (local_24 & 0x8000) != 0 && local_18 != 0) {
        g_TER1_KELP_FARM_SPRITE.draw(
            buffer, sprite_transparent(g_TER1_KELP_FARM_SPRITE), a9, a10);
    }

    // THE BASE/ROAD/MAGTUBE PASS (sites 18-20 and the sensor-array
    // highlight): base_at's visibility first, then the eight neighbours'
    // road bits pick the edge receivers.
    local_30b = a6 & 0x20;
    if (local_30b != 0) {
        local_88 = base_at(a7, a8);
        if ((int)local_88 > -1 && a4 > -1 &&
            (Bases[(int)local_88].faction_id_current != a4 &&
             (Bases[(int)local_88].visibility &
              (uint8_t)(1 << (bVar10 & 0x1f))) == 0)) {
            local_88 = -1;
        }
        if (local_18 == 0 &&
            ((local_24 & 4) != 0 || (int)local_88 > -1)) {
            if (((local_24 & 8) != 0) ||
                (bVar23 = false, (int)local_88 > -1)) {
                bVar23 = true;
            }
            local_94 = 0;
            local_2c = 0;
            {
                int k = 0;
                do {
                    iVar5 = RadiusBaseX[k] + a7;
                    if ((MapIsFlat & 1) == 0) {
                        if (iVar5 < 0) {
                            iVar5 = MapLongitudeBounds + iVar5;
                        } else if (MapLongitudeBounds <= iVar5) {
                            iVar5 = iVar5 - MapLongitudeBounds;
                        }
                    }
                    local_34b = RadiusBaseY[k] + a8;
                    if ((int)local_34b > -1 &&
                        (int)local_34b < MapLatitudeBounds && iVar5 > -1 &&
                        iVar5 < MapLongitudeBounds) {
                        iVar19 = *MapRowStride * (int)local_34b + (iVar5 >> 1);
                        road_neighbor = &map_tiles()[iVar19];
                        if (0x5f < (road_neighbor->climate & 0xe0)) {
                            if (a4 < 0) {
                                local_44 = road_neighbor->bit;
                            } else {
                                local_44 = a4 == 0
                                    ? map_tiles()[iVar19].bit2
                                    : map_tiles()[iVar19].bit_visible[a4 - 1];
                            }
                            iVar5 = base_at(iVar5, (int)local_34b);
                            if (iVar5 > -1 && a4 > -1 &&
                                (Bases[iVar5].faction_id_current != a4 &&
                                 (Bases[iVar5].visibility &
                                  (uint8_t)(1 << (bVar10 & 0x1f))) == 0)) {
                                iVar5 = -1;
                            }
                            if (((local_44 & 4) != 0) || iVar5 > -1) {
                                local_94 = 1;
                                overlay_uv[0].x = 0;
                                overlay_uv[0].y = 0;
                                overlay_uv[1].x = 0x37;
                                overlay_uv[1].y = 0;
                                overlay_uv[2].x = 0x37;
                                overlay_uv[2].y = 0x37;
                                overlay_uv[3].x = 0;
                                overlay_uv[3].y = 0x37;
                                if (bVar23 &&
                                    (((local_44 & 8) != 0) ||
                                     iVar5 > -1)) {
                                    g_MAGTUBE_TEXTURES
                                        [1 + ((k + 2) & 7)]
                                            .draw_trans(
                                                buffer,
                                                MapCornerScratch,
                                                local_b8, 4,
                                                overlay_uv);
                                    local_2c = 1;
                                } else {
                                    g_ROAD_TEXTURES[1 + ((k + 2) & 7)]
                                        .draw_trans(
                                            buffer,
                                            MapCornerScratch,
                                            local_b8, 4,
                                            overlay_uv);
                                }
                            }
                        }
                    }
                    k = k + 1;
                } while (k < 8);
            }
            if ((int)local_88 < 0 &&
                ((bVar23 && local_2c == 0) ||
                 local_94 == 0)) {
                overlay_uv[0].x = 0;
                overlay_uv[0].y = 0;
                overlay_uv[1].x = 0x37;
                overlay_uv[1].y = 0;
                overlay_uv[2].x = 0x37;
                overlay_uv[2].y = 0x37;
                overlay_uv[3].x = 0;
                overlay_uv[3].y = 0x37;
                if (bVar23) {
                    g_MAGTUBE_TEXTURES[0].draw_trans(
                        buffer,
                        MapCornerScratch,
                        local_b8, 4,
                        overlay_uv);
                } else {
                    g_ROAD_TEXTURES[0].draw_trans(
                        buffer,
                        MapCornerScratch,
                        local_b8, 4,
                        overlay_uv);
                }
            }
        }
    }
    if ((local_30b != 0 && (local_24 & 0x8000) != 0 && -8 < a3) ||
        (a6 & 0x1000) != 0) {
        crop_diff = crop_yield(territory_faction, 0xffffffff, a7, a8, 0);
    }

    // THE BONUS/RESOURCE SPRITES (sites 23/24).
    if ((a6 & 0x200) != 0) {
        iVar5 = bonus_at(a7, a8, 0);
        if (iVar5 != 0 && -8 < a3) {
            if (iVar5 == 4) {
                g_TER1_LANDMARK_RESOURCE_SPRITES
                    [4 + (local_18 != 0)]
                        .draw(buffer, sprite_transparent(g_TER1_LANDMARK_RESOURCE_SPRITES [4 + (local_18 != 0)]),
                              a9, a10);
            } else {
                g_TER1_SEA_LAND_RESOURCE_SPRITES
                    [(((local_14 + a8 & 1U) - 4) +
                      ((uint32_t)(local_18 != 0) + iVar5 * 2) * 2)]
                        .draw_dest(
                            buffer,
                            sprite_transparent(
                                g_TER1_SEA_LAND_RESOURCE_SPRITES
                                    [(((local_14 + a8 & 1U) - 4) +
                                      ((uint32_t)(local_18 != 0) +
                                       iVar5 * 2) *
                                          2)]),
                            a9, a10, ShadowColorTable);
            }
        }
    }
    if (local_30b != 0) {
        // THE IMPROVEMENT SPRITES (sites 25-35), each gated by its tile
        // flag; receivers are the g_TER1_* band in flag order.
        if ((local_24 & 0x800) != 0 && -8 < a3 && local_18 == 0) {
            g_TER1_BUNKER_SPRITE.draw(
                buffer, sprite_transparent(g_TER1_BUNKER_SPRITE), a9, a10);
        }
        if ((local_24 & 0x40000) != 0 && -8 < a3 && local_18 == 0) {
            g_TER1_AIRBASE_SPRITE.draw(
                buffer, sprite_transparent(g_TER1_AIRBASE_SPRITE), a9,
                a10);
        }
        if ((local_24 & 0x80000000) != 0 && -8 < a3) {
            g_TER1_SENSOR_ARRAY_SPRITE.draw_dest(
                buffer, sprite_transparent(g_TER1_SENSOR_ARRAY_SPRITE),
                a9, a10, ShadowColorTable);
        }
        if ((local_24 & 0x8000) != 0 && -8 < a3 && local_18 == 0) {
            uVar12 = local_24 & 0x8000;
            if ((local_24 & 0x80000) == 0) {
                iVar5 = crop_diff + -1;
                if (iVar5 < 0) {
                    iVar5 = 0;
                } else if (3 < iVar5) {
                    iVar5 = 3;
                }
                g_TER1_FARM_SPRITES[iVar5].draw(
                    buffer,
                    sprite_transparent(g_TER1_FARM_SPRITES[iVar5]), a9,
                    a10);
            } else {
                iVar5 = crop_diff + -1;
                if (iVar5 < 0) {
                    iVar5 = 0;
                } else if (3 < iVar5) {
                    iVar5 = 3;
                }
                g_TER1_SOIL_ENRICHER_SPRITES[iVar5].draw(
                    buffer, sprite_transparent(g_TER1_SOIL_ENRICHER_SPRITES[iVar5]),
                    a9, a10);
            }
        }
        if ((local_24 & 0x10) != 0 && -8 < a3) {
            if (local_18 == 0) {
                g_TER1_MINE_SPRITE.draw(
                    buffer, sprite_transparent(g_TER1_MINE_SPRITE), a9,
                    a10);
            } else if (ExpansionEnabled != 0 || (int)local_8c == 2) {
                g_TER1_MINING_PLATFORM_SPRITE.draw(
                    buffer,
                    sprite_transparent(g_TER1_MINING_PLATFORM_SPRITE), a9,
                    a10);
            }
        }
        if ((local_24 & 0x40) != 0 && -8 < a3) {
            if (local_18 == 0) {
                g_TER1_SOLAR_COLLECTOR_SPRITE.draw(
                    buffer,
                    sprite_transparent(g_TER1_SOLAR_COLLECTOR_SPRITE), a9,
                    a10);
            } else if (ExpansionEnabled != 0 || (int)local_8c == 2) {
                g_TER1_TIDAL_HARNESS_SPRITE.draw(
                    buffer,
                    sprite_transparent(g_TER1_TIDAL_HARNESS_SPRITE), a9,
                    a10);
            }
        }
        if ((local_24 & 0x400000) != 0 && -8 < a3) {
            g_TER1_CONDENSER_SPRITE.draw(
                buffer, sprite_transparent(g_TER1_CONDENSER_SPRITE), a9,
                a10);
        }
        if ((local_24 & 0x800000) != 0 && -8 < a3) {
            g_TER1_ECHELON_MIRROR_SPRITE.draw(
                buffer, sprite_transparent(g_TER1_ECHELON_MIRROR_SPRITE),
                a9, a10);
        }
        if ((local_24 & 0x1000000) != 0 && -8 < a3) {
            uVar16b = sprite_transparent(g_TER1_BOREHOLE_SPRITE);
            if ((tile->bit2 & 0x80001000) == 0x1000) {
                uVar16b =
                    sprite_transparent(g_TER1_BOREHOLE_CLUSTER_SPRITE);
                g_TER1_BOREHOLE_CLUSTER_SPRITE.draw(buffer, uVar16b, a9,
                                                    a10);
            } else {
                g_TER1_BOREHOLE_SPRITE.draw(buffer, uVar16b, a9, a10);
            }
        }
        if ((local_24 & 0x8000) != 0 && -8 < a3 && local_18 == 0 &&
            4 < crop_diff && (local_24 & 0x80000) != 0) {
            g_TER1_SOIL_ENRICHER_SPRITES[0].draw(
                buffer, sprite_transparent(g_TER1_SOIL_ENRICHER_SPRITES[0]),
                a9, a10);
        }
        {
            uVar12 = tile->bit2;
            if ((uVar12 & 0x80002000) == 0x2000) {
                iVar5 = (int)uVar12 >> 0x18;
                if (0x1ffffff < (int)(uVar12 & 0xff000000)) {
                    if ((int)(uVar12 & 0xff000000) < 0x4000001) {
                        goto nexus_done;
                    }
                    iVar5 = iVar5 + -3;
                }
                g_TER1_MANIFOLD_NEXUS_SPRITES[iVar5].draw(
                    buffer, sprite_transparent(g_TER1_MANIFOLD_NEXUS_SPRITES[iVar5]),
                    a9, a10);
            }
        }
nexus_done:;
        {
            uVar12 = tile->bit2;
            if ((uVar12 & 0x80004000) == 0x4000 &&
                ExpansionEnabled != 0) {
                Sprite *wreck;
                uVar16b = uVar12 & 0xff000000;
                if (uVar16b == 0) {
                    wreck = goody_at(a7, a8) == 0
                                ? &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES
                                      [0]
                                : &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[0];
                } else if ((int)uVar16b < 0x8000000) {
                    wreck = &g_TER1WRECK_UNITY_WRECKAGE_SPRITES
                        [(int)uVar16b >> 0x18];
                } else if (uVar16b == 0x8000000) {
                    wreck = goody_at(a7, a8) == 0
                                ? &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES
                                      [1]
                                : &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[8];
                } else if (uVar16b == 0xa000000) {
                    wreck = goody_at(a7, a8) == 0
                                ? &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES
                                      [2]
                                : &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[9];
                } else if (uVar16b == 0xc000000) {
                    wreck = &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[10];
                } else if (uVar16b == 0xf000000) {
                    wreck = &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[11];
                } else if (uVar16b == 0x10000000) {
                    wreck = &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[12];
                } else if (uVar16b == 0x13000000) {
                    wreck = goody_at(a7, a8) == 0
                                ? &g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES
                                      [3]
                                : &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[13];
                } else {
                    if (uVar16b != 0x14000000) {
                        goto wreck_done;
                    }
                    wreck = &g_TER1WRECK_UNITY_WRECKAGE_SPRITES[14];
                }
                wreck->draw(buffer, sprite_transparent(*wreck), a9,
                            a10);
            }
        }
wreck_done:;
        {
            uVar12 = tile->bit2;
            if ((uVar12 & 0x80008000) == 0x8000 &&
                ExpansionEnabled != 0 &&
                (int)(uVar12 & 0xff000000) < 0x6000000) {
                g_FOSSIL_FIELD_RIDGE_SPRITES[(int)uVar12 >> 0x18].draw(
                    buffer, sprite_transparent(g_FOSSIL_FIELD_RIDGE_SPRITES[(int)uVar12 >> 0x18]),
                    a9, a10);
            }
        }
    }

    // THE POD/MONOLITH/GRID OVERLAYS (sites 38-41).
    if ((a6 & 0x200) != 0 && -8 < a3) {
        iVar5 = goody_at(a7, a8);
        if (iVar5 != 0) {
            uVar12 = tile->bit2;
            if ((uVar12 & 0x80004000) != 0x4000 ||
                ((uVar12 = uVar12 & 0xff000000, uVar12 != 0 &&
                  uVar12 != 0x8000000) &&
                 (uVar12 != 0xa000000 && uVar12 != 0x13000000))) {
                g_TER1_UNITY_POD_SPRITES
                    [local_18 + ((int)(a7 * 7 + a8 * 5) % 3) * 2]
                        .draw(buffer, sprite_transparent(g_TER1_UNITY_POD_SPRITES [local_18 + ((int)(a7 * 7 + a8 * 5) % 3) * 2]),
                              a9, a10);
            }
        }
        if ((tile->bit & 0x2000) != 0) {
            g_TER1_MONOLITH_SPRITE.draw_dest(
                buffer, sprite_transparent(g_TER1_MONOLITH_SPRITE), a9,
                a10, ShadowColorTable);
        }
    }
    if (local_18 == 0) {
        if ((a6 & 8) != 0) {
            g_TER1_WHITE_ORG_YEL_TILE_SPRITES
                [local_90 + (tile->climate & 7) * 2]
                    .draw(buffer, sprite_transparent(g_TER1_WHITE_ORG_YEL_TILE_SPRITES [local_90 + (tile->climate & 7) * 2]),
                          a9, a10);
        }
        if (local_20 == 0 && (a6 & 0x10) != 0) {
            if (local_90 != 0 && (a6 & 8) != 0) {
                local_90 = local_90 + 1;
            }
            g_TER1_BOTTOM_LEFT_TILE_SPRITES
                [local_90 +
                 ((tile->climate & 0x18) >> 3) * 3]
                    .draw(buffer, sprite_transparent(g_TER1_BOTTOM_LEFT_TILE_SPRITES [local_90 + ((tile->climate & 0x18) >> 3) * 3]),
                          a9, a10);
        }
    }

    // THE EDITOR READOUTS: the window's own Buffer (vbtable walk), the
    // shared StringTemp scratch and window->font_b_.
    if (window != 0) {
        Buffer *const surface = window;
        int width = local_78;
        if ((a6 & 4) != 0) {
            StringTemp[0] = 0;
            width = local_78;
            if ((tile->bit & 0x1000) == 0) {
                uVar12 = (uint32_t)(tile->val2 >> 4);
                if (uVar12 == 0) {
                    uVar12 = (uint32_t)world_site(a7, a8, 0);
                    site_set(a7, a8, uVar12);
                }
            } else {
                uVar12 = 0;
            }
            _itoa(uVar12, numbuf, 10);
            strcat(StringTemp, numbuf);
            width = local_78;
            local_a8 = a9;
            local_a4 = a10;
            local_9c = a10 + local_78 * 2;
            local_a0 = a9 + local_78 * 4;
            surface->set_text_color(
                ((0x5f < (tile->climate & 0xe0)) - 1 & 0xfffffffd) + 0xff,
                0, 1, 1);
            surface->set_font(&window->font_b_, 0, 0, 0);
            if (uVar12 != 0 && local_18 == 0) {
                sVar9 = strlen(StringTemp);
                RECT rect;
                rect.left = local_a8;
                rect.top = local_a4;
                rect.right = local_a0;
                rect.bottom = local_9c;
                surface->write_cent_l(StringTemp, &rect, sVar9);
            }
        }
        if ((a6 & 0x4000) != 0) {
            StringTemp[0] = 0;
            _itoa(Paths.get(a7, a8), numbuf, 10);
            strcat(StringTemp, numbuf);
            surface->set_text_color(0xff, 0, 1, 1);
            surface->set_font(&window->font_b_, 0, 0, 0);
            local_a8 = a9;
            local_a4 = a10;
            local_a0 = a9 + width * 4;
            local_9c = a10 + width * 2;
            sVar9 = strlen(StringTemp);
            {
                RECT rect;
                rect.left = local_a8;
                rect.top = local_a4;
                rect.right = local_a0;
                rect.bottom = local_9c;
                surface->write_cent_l(StringTemp, &rect, sVar9);
            }
        }
        // THE LOCK-FACTION READOUT, drawn twice in the image (both blocks
        // identical, 0x466870 and 0x46699C - kept faithfully).
        for (int pass = 0; pass < 2; pass++) {
            if ((a6 & 0x80000) != 0 &&
                (tile->val3 & 0x38) != 0) {
                local_a8 = a9;
                local_a4 = a10;
                local_a0 = a9 + width * 4;
                local_9c = a10 + width * 2;
                surface->set_text_color(
                    InfoFactionTextColors[(tile->val3 & 0x38) >> 3], 0,
                    1, 1);
                surface->set_font(&window->font_b_, 0, 0, 0);
                StringTemp[0] = 0;
                _itoa((tile->val3 & 0x38) >> 3, numbuf, 10);
                strcat(StringTemp, numbuf);
                sVar9 = strlen(StringTemp);
                RECT rect;
                rect.left = local_a8;
                rect.top = local_a4;
                rect.right = local_a0;
                rect.bottom = local_9c;
                surface->write_cent_l(StringTemp, &rect, sVar9);
            }
        }
    }
    result = 1;

LAB_00466c32:;
    // THE COMMON EXIT.
    if (0 < MapWinCaptionScale && window != 0) {
        window->field_8_ = 99999;
    }
    SpriteScaleNumerator = 1;
    SpriteScaleDenominator = 1;
    TextureLightFloor = 0xffffff00;
    TextureLightCeiling = 0x100;
    return result;
}

/*
Purpose: Render one map tile - terrain first, then vehicles and overlays.
         The tile coordinates enter as (x, y) and are transformed IN PLACE by
         tile_to_pixel through pointers to their own parameter homes, so the
         pre-call pair has to survive in registers (ebx/edi in the image) to
         reach gen_terrain_poly as arguments 7/8 while the post-call pair is
         read back from the same homes as arguments 9/10 - and to gate the
         vehicle/overlay tail, which the ORIGINAL pair does, not the pixel
         pair. The surface handed to gen_terrain_poly is the window's own
         Buffer base; converting `this` to it walks the vbtable to the
         GraphicWin virtual base and adds 0x444, and VC6 emits the null
         select around that walk itself (measured on a standalone probe:
         every spelling of the plain `Buffer *surface = this;` upcast
         produces the test/jne/xor-jmp/lea shape the image carries).
// ORIGINAL: 0x00468C20 ?gen_tile@MapWin@@QAEXHHHH@Z 0x00468C20-0x00468CC5 FILE
// size      165 bytes
// prototype void (__thiscall ?gen_tile@MapWin@@QAEXHHHH@Z)(MapWin* this, int, int, int, int)
// callers   0   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462F00 0x004632D0 0x00466CA0 0x004688E0
// TRIED: 53/71 instructions match under /c /O2 /Gy /GR- /Oy- /GX; the whole
//            body agrees - prologue, in-place transform, the ten-argument
//            forward, bounds gate and both tail calls - except ONE schedule
//            slot: the image preloads field_1DD70_ into ECX at 0x00468C3F,
//            BEFORE the Buffer conversion's null select, and pushes py/px as
//            load-push/load-push pairs through EDX; this spelling computes
//            the select first (EAX, the image's own destination and walk
//            registers) and sinks the field_1DD70_ load to between the two
//            pushes. Measured alternatives: a const or assigned flags local
//            declared before the conversion materializes the load early but
//            swaps the pair (flags EAX / select ECX, 51/71); a window local
//            chain, an inline conversion, and a forced-inline shared helper
//            all reproduce the select-first form. No spelling found that
//            loads field_1DD70_ into ECX ahead of an EAX select; recorded as
//            a scheduling plateau, not chased further at this budget.
*/
void MapWin::gen_tile(int x, int y, int a3, int a4) {
    const int tile_x = x;
    const int tile_y = y;
    tile_to_pixel(x, y, &x, &y);
    gen_terrain_poly(this, this, field_1DD98_, a3, -1, field_1DD70_,
                     tile_x, tile_y, x, y);
    if (tile_y >= 0 && tile_y < MapLatitudeBounds &&
        tile_x >= 0 && tile_x < MapLongitudeBounds && a4 != 0) {
        draw_vehicles(tile_x, tile_y, x, y - field_1DDC0_, a3);
        gen_overlays(tile_x, tile_y);
    }
}

// The pinned image catalogue calls this entry sub_4688E0; the source name is
// inferred from both typed MapWin callers (gen_tile at 0x00468CB9 and gen_map
// at 0x0046A221), not from an original decorated name. Catalogued code span:
// [0x004688E0, 0x00468BC5). Its jump/selector data at 0x00468BC8..0x00468C12
// is adjacent data outside that span and is intentionally not source-mapped
// here. See build/recovery/mapwin-gen-overlays-oct6/REPORT.md for the pinned
// hash, full call chains, data interpretation, and remaining identity limits.
// ORIGINAL: 0x004688E0 sub_4688e0 0x004688E0-0x00468BC5
// RECOVERY_BINDING: v1 {"evidence": ["pinned:terranx_original.exe:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf", "build/recovery/mapwin-gen-overlays-oct6/REPORT.md:catalogue-chunk-caller-storage", "build/recovery/mapwin-gen-overlays-oct6/INDEPENDENT-REVIEW-OCT6.md:original-0x4689EF-per-goal-reset", "native:COFF:218f67bf0421f47c7bbcbabcfa583b5d325520e1eff7580df081c758b94aa1cc:752bytes-39references"], "reason": "Complete MapWin goal overlays through genuine PlayerData Goal arrays, mapped game state and inherited Buffer APIs. Local name is inferred from two receiver-backed callers, not an original decorated symbol. Catalogue/chunk, receiver accesses and ret8 support extent; adjacent selector tables stay outside. Preserve unchecked strings, signed-short priorities, duplicate mutation and per-renderable-goal reset/global updates."}
// symbol    ?gen_overlays@MapWin@@QAEXHH@Z
void MapWin::gen_overlays(int x, int y) {
    if ((field_1DD70_ & 0x2000U) == 0) {
        return;
    }

    int pixel_x;
    int pixel_y;
    tile_to_pixel(x, y, &pixel_x, &pixel_y);

    RECT text_area;
    text_area.left = pixel_x;
    text_area.top = pixel_y;
    text_area.right = pixel_x + static_cast<int>(field_1DDB4_);
    text_area.bottom = pixel_y + static_cast<int>(field_1DDB8_);

    const bool omniscient =
        (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
    Buffer *const canvas = static_cast<Buffer *>(this);

    // The original visits factions 1 through 7. In ordinary view it keeps
    // only the local faction; with omniscience it walks all seven rows.
    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        if (!omniscient && faction != ConsoleGlobal.faction_id_) {
            continue;
        }

        Goal *const goals = PlayersData[faction].goals;
        for (int goal_index = 0; goal_index < MaxGoalsNum; ++goal_index) {
            Goal &goal = goals[goal_index];
            if (goal.x != x || goal.y != y || goal.type < 0 ||
                goal.priority == 0) {
                continue;
            }

            // Negative priorities trigger the image's duplicate check. It
            // suppresses a later same-type, same-coordinate record by
            // mutating that current Goal's type to -1 in place. Preserve the
            // mutation and the negative-priority-only condition.
            if (goal.priority < 0) {
                for (int previous = 0; previous < goal_index; ++previous) {
                    const Goal &earlier = goals[previous];
                    if (earlier.x == x && earlier.y == y &&
                        earlier.type == goal.type) {
                        goal.type = -1;
                    }
                }
                if (goal.type < 0) {
                    continue;
                }
            }

            // The image resets the shared text buffer for each renderable
            // record, then prepends the faction noun only in omniscient view.
            StringTemp[0] = '\0';
            if (omniscient) {
                GenderDefault = Players[faction].noun_gender;
                PluralityDefault = Players[faction].is_noun_plural;
                strcat(StringTemp, Players[faction].noun_faction);
            }

            // The original adjacent selector tables map only these Goal type
            // values to a one-character label and color. Other nonnegative
            // types, including values above 0x29, keep the default color and
            // have no label. Keep numeric identities rather than inventing
            // names for the enum values.
            const char *label = 0;
            int color = 0xFF;
            switch (goal.type) {
            case 0:
                label = "a";
                color = 0xF9;
                break;
            case 2:
                label = "d";
                color = 0xFB;
                break;
            case 3:
                label = "f";
                color = 0xFA;
                break;
            case 6:
                label = "c";
                color = 0xFE;
                break;
            case 8:
                label = "n";
                color = 0xFC;
                break;
            case 9:
                label = "^";
                color = 0xFF;
                break;
            case 41:
                label = "s";
                color = 0xFD;
                break;
            default:
                break;
            }

            if (label != 0) {
                strcat(StringTemp, label);
            }

            int priority = goal.priority;
            if (priority < 0) {
                priority = -priority;
            }
            char number[10];
            _itoa(priority, number, 10);
            strcat(StringTemp, number);

            canvas->set_text_color(color, 0, 1, 1);
            canvas->set_font(&font_b_, 0, 0, 0);
            canvas->write_cent_l(StringTemp, &text_area,
                                 static_cast<int>(strlen(StringTemp)));
        }
    }
}

// Original0x00467DE0 draws base labels and map landmarks in the visible map
// rectangle. Both data collections use their source-owned records and counts.
// ORIGINAL: 0x00467DE0 ?draw_base_names@MapWin@@QAEXXZ 0x00467DE0-0x004688E0 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00467de0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// TRIED: Ghidra transcription with `this` restored on every Buffer/ Strings call Ghidra dropped (set_clip/set_font/set_text_color/ write_cent_l via a virtual-base offset `*(int*)(*(int*)this+4)`, Strings::get via fixed g_009b90d8); diverges at insn #6 (mov vs push), i.e. within the vtable/vbase-offset prologue read.
void MapWin::draw_base_names() {
    if (MapBaseDisplayOverride == 0 && ControlTurnPhase == 0 &&
        (GameDataStateStorage.fields_.game_rules_ & 0x80U) == 0) {
        return;
    }

    Buffer *const canvas = static_cast<Buffer *>(this);
    RECT saved_clip = canvas->rect1_;
    canvas->set_clip(&canvas->rect2_);
    draw_base_dest(0);

    const int faction = ConsoleGlobal.faction_id_;
    const uint32_t faction_bit = 1U << (faction & 31);
    const bool omniscient = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
    const bool show_base_names =
        (field_1DD70_ & 0x800U) != 0 &&
        (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_MAP_SHOW_BASE_NAMES) != 0;

    if (show_base_names && field_1DD98_ < -10) {
        canvas->set_clip(&saved_clip);
        return;
    }
    if (show_base_names) {
        canvas->set_font(&font_a_, 0, 0, 0);
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            const Base &base = Bases[base_id];
            const int tile_x = base.x;
            const int tile_y = base.y;
            if (tile_y < field_1DDA8_ ||
                tile_y >= field_1DDA8_ + field_1DDD8_ + field_1DDD0_) {
                continue;
            }

            int visible_x = tile_x;
            if ((MapIsFlat & 1) == 0) {
                if (visible_x < field_1DDA4_) {
                    visible_x += MapLongitudeBounds;
                }
                if (visible_x >= field_1DDA4_ + field_1DDD4_ + field_1DDCC_) {
                    visible_x -= MapLongitudeBounds;
                }
            }
            if (visible_x < field_1DDA4_ ||
                visible_x >= field_1DDA4_ + field_1DDD4_ + field_1DDCC_) {
                continue;
            }

            const Map *const tile = map_loc(tile_x, tile_y);
            if (!omniscient && base.faction_id_current != faction &&
                (base.visibility & faction_bit) == 0) {
                continue;
            }

            int pixel_x;
            int pixel_y;
            tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
            char text[256];
            text[0] = '\0';
            strcat(text, base.name_string);
            if ((base.state & 0x10U) != 0 &&
                base.faction_id_current == faction) {
                strcat(text, " (*)");
            }
            canvas->set_text_color(InfoFactionTextColors[base.faction_id_current],
                                   InfoFactionShadowColors[base.faction_id_current],
                                   1, 1);
            if ((GameDataStateStorage.fields_.game_state_ & 0x1000U) != 0 &&
                (omniscient || (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & faction_bit) == 0)) {
                const PlayerData &owner_data =
                    PlayersData[base.faction_id_current];
                if (base.x == owner_data.x_target &&
                    base.y == owner_data.y_target) {
                    canvas->set_text_color(0xFF, 0, 1, 1);
                }
            }
            canvas->write_cent_l(text, pixel_x + field_1DDBC_,
                                 pixel_y + (field_1DDC8_ * 7) / 2,
                                 strlen(text));

            const bool show_region_plan = (GameDataStateStorage.fields_.game_state_ & 0x1000U) != 0 &&
                omniscient && field_1DD98_ >= -3;
            if (show_region_plan) {
                const int region = tile->region;
                const int plan = PlayersData[base.faction_id_current]
                                     .region_base_plan[region];
                char detail[256];
                detail[0] = '\0';
                strcat(detail, "(");
                strcat(detail, StringTable.get(
                    reinterpret_cast<int>(PlansShortName[plan])));
                strcat(detail, ": ");
                const int production_id = base.queue_production_id[0];
                const char *production_name = production_id < 0
                    ? StringTable.get(reinterpret_cast<int>(
                          Facility[-production_id].name))
                    : VehPrototypes[production_id].veh_name;
                strcat(detail, production_name);
                strcat(detail, ")");
                canvas->write_cent_l(detail,
                    pixel_x + field_1DDBC_,
                    pixel_y + (field_1DDC8_ * 7) / 2 +
                        font_a_.line_advance(), strlen(detail));
            } else {
                const bool production_detail =
                    (GameDataStateStorage.fields_.game_more_preferences_ & MPREF_MAP_SHOW_PROD_WITH_BASE_NAMES) != 0 ||
                    (field_1DD70_ & 0x08000000U) != 0;
                if (!production_detail) {
                    if (is_objective(base_id) &&
                        (GameDataStateStorage.fields_.game_rules_ & 0x20000000U) == 0) {
                        LPSTR objective = label_get(898);
                        if (objective) {
                            char detail[256];
                            detail[0] = '\0';
                            strcat(detail, objective);
                            CharUpperA(detail);
                            canvas->write_cent_l(detail,
                                pixel_x + field_1DDBC_,
                                pixel_y + (field_1DDC8_ * 7) / 2 +
                                    font_a_.line_advance(),
                                strlen(detail));
                        }
                    }
                } else if (base.faction_id_current == faction ||
                           spying(base.faction_id_current) != 0) {
                    const int production_id = base.queue_production_id[0];
                    LPSTR production_name = 0;
                    if (production_id < 0 &&
                        -production_id < MaxFacilityNum) {
                        production_name = StringTable.get(
                            reinterpret_cast<int>(Facility[-production_id].name));
                    } else if (production_id >= 0 &&
                               production_id < MaxVehProtoNum) {
                        production_name = VehPrototypes[production_id].veh_name;
                    }
                    if (production_name) {
                        char detail[256];
                        detail[0] = '\0';
                        const bool objective = is_objective(base_id) != 0;
                        if (objective && (GameDataStateStorage.fields_.game_rules_ & 0x20000000U) == 0) {
                            LPSTR name = label_get(898);
                            if (name) {
                                strcat(detail, name);
                                strcat(detail, ": ");
                                CharUpperA(detail);
                            }
                        } else if ((base.governor & 0x80000001U) ==
                                   0x80000001U) {
                            int descriptor_label = 649;
                            if ((base.governor & 0x01000000U) != 0)
                                descriptor_label = 650;
                            else if ((base.governor & 0x02000000U) != 0)
                                descriptor_label = 651;
                            else if ((base.governor & 0x04000000U) != 0)
                                descriptor_label = 652;
                            else if ((base.governor & 0x08000000U) != 0)
                                descriptor_label = 653;
                            LPSTR descriptor = label_get(descriptor_label);
                            if (descriptor) {
                                strcat(detail, descriptor);
                                strcat(detail, ": ");
                            }
                        }
                        strcat(detail, "(");
                        strcat(detail, production_name);
                        strcat(detail, ")");
                        canvas->write_cent_l(detail,
                            pixel_x + field_1DDBC_,
                            pixel_y + (field_1DDC8_ * 7) / 2 +
                                font_a_.line_advance(),
                            strlen(detail));
                    }
                } else {
                    const bool objective = is_objective(base_id) != 0;
                    if (objective && (GameDataStateStorage.fields_.game_rules_ & 0x20000000U) == 0) {
                        LPSTR objective_name = label_get(898);
                        if (objective_name) {
                            char detail[256];
                            detail[0] = '\0';
                            strcat(detail, objective_name);
                            CharUpperA(detail);
                            canvas->write_cent_l(detail,
                                pixel_x + field_1DDBC_,
                                pixel_y + (field_1DDC8_ * 7) / 2 +
                                    font_a_.line_advance(),
                                strlen(detail));
                        }
                    }
                }
            }
        }
    }

    int landmark_intensity = 1;
    if (field_1DD98_ > 0) {
        landmark_intensity = MapWinCaptionScale > 0
            ? MapWinCaptionScale / MapWinCaptionDivisor
            : (field_1DD98_ + 16) / 16;
    }
    canvas->set_text_color(0xFF, 0, landmark_intensity,
                           landmark_intensity);
    canvas->set_font(&font_c_, 0, 0, 0);
    for (int index = 0; index < MapLandmarkCount; ++index) {
        Landmark &landmark = MapLandmark[index];
        if (landmark.y < field_1DDA8_ ||
            landmark.y >= field_1DDA8_ + field_1DDD8_ + field_1DDD0_) {
            continue;
        }
        int visible_x = landmark.x;
        if ((MapIsFlat & 1) == 0) {
            if (visible_x < field_1DDA4_) visible_x += MapLongitudeBounds;
            if (visible_x >= field_1DDA4_ + field_1DDD4_ + field_1DDCC_) {
                visible_x -= MapLongitudeBounds;
            }
        }
        if (visible_x < field_1DDA4_ ||
            visible_x >= field_1DDA4_ + field_1DDD4_ + field_1DDCC_) {
            continue;
        }
        const Map *const tile = map_loc(landmark.x, landmark.y);
        const bool visible = omniscient ||
            (PlayersData[faction].flags & 0x200U) != 0 ||
            (tile->visibility & faction_bit) != 0;
        if (!visible || ((tile->bit & 1U) != 0 &&
                         (tile->val2 & 0x0FU) <= 7U)) {
            continue;
        }
        int pixel_x;
        int pixel_y;
        tile_to_pixel(landmark.x, landmark.y, &pixel_x, &pixel_y);
        canvas->write_cent_l(landmark.name,
                             pixel_x + field_1DDBC_,
                             pixel_y + (field_1DDC8_ * 7) / 2,
                             strlen(landmark.name));
    }
    canvas->set_clip(&saved_clip);
}

namespace {
void mapwin_add_context_label(PullDown &menu, int label_byte_offset, int id) {
    LPSTR text = label_get(label_byte_offset / 4);
    if (text) menu.add_item(id, text);
}

void mapwin_add_parenthesized_label(PullDown &menu, int label_byte_offset,
                                    int id) {
    LPSTR source = label_get(label_byte_offset / 4);
    if (!source) return;
    char text[512];
    text[0] = '\0';
    strcat(text, "(");
    strcat(text, source);
    strcat(text, ")");
    menu.add_item(id, text);
}

void mapwin_add_base_production(PullDown &menu, const Base &base, int id) {
    const int production = base.queue_production_id[0];
    LPSTR production_name = 0;
    if (production < 0 && -production < MaxFacilityNum) {
        production_name = StringTable.get(
            reinterpret_cast<int>(Facility[-production].name));
    } else if (production >= 0 && production < MaxVehProtoNum) {
        production_name = VehPrototypes[production].veh_name;
    }
    if (!production_name) return;
    char text[512];
    text[0] = '\0';
    strcat(text, "(");
    strcat(text, production_name);
    strcat(text, ")");
    menu.add_item(id, text);
}
}

// Original0x0046C0D0 builds the quick/context menu from live map owners, then
// revalidates the selected base, vehicle and cursor before dispatching an
// action. PullDown and MenuEntry own the modal menu and nested submenus.
// ORIGINAL: 0x0046C0D0 ?right_menu@MapWin@@QAEXHHHH@Z 0x0046C0D0-0x0046D552;0x00655944-0x00655959 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0046c0d0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// TRIED: full transcription of both menu-build paths (quick 6-item and the full context menu) and the whole 3-level action dispatch (39 item ids across 4 jump-table ranges); MISMATCH #15 is in the SEH/PullDown-construction prologue, not the menu logic - coverage is close to 100% of the original's ~5270 bytes. A few multi-arg call orderings (e.g. lock_veh) were derived from push order rather than re-verified against a second source.
void MapWin::right_menu(int tile_x, int tile_y, int screen_x, int screen_y) {
    client_to_screen(&screen_x, &screen_y);
    int base_id = base_at(tile_x, tile_y);
    int vehicle_id = veh_at(tile_x, tile_y);
    int selected_vehicle = ConsoleGlobal.selected_vehicle_;
    PullDown menu;
    menu.init(0);

    if (field_1DD78_ == 1) {
        mapwin_add_context_label(menu, 0xC04, 0x320);
        menu.add_separator(0x63);
        mapwin_add_context_label(menu, 0xC68, 0x323);
        mapwin_add_context_label(menu, 0xC6C, 0x324);
        mapwin_add_context_label(menu, 0xC70, 0x325);
        menu.add_separator(0x63);
        mapwin_add_context_label(menu, 0x910, 0x321);
        mapwin_add_context_label(menu, 0x914, 0x322);
        menu.show(screen_x, screen_y, pop_expire);
        if ((GameDataStateStorage.fields_.game_state_ & 0x800U) != 0) return;

        const int item_id = menu.get_selected_id();
        if (base_at(tile_x, tile_y) != base_id ||
            veh_at(tile_x, tile_y) != vehicle_id ||
            ConsoleGlobal.selected_vehicle_ != selected_vehicle) {
            return;
        }
        if (IsMultiplayerNet != 0) {
            base_at(tile_x, tile_y);
            veh_at(tile_x, tile_y);
            selected_vehicle = ConsoleGlobal.selected_vehicle_;
        }
        ConsoleGlobal.get_mouse_pos(&screen_x, &screen_y);
        client_to_screen(&screen_x, &screen_y);
        switch (item_id) {
        case 0x320:
            WorldClimateWorldWin.set_center(tile_x, tile_y);
            WorldClimateWorldWin.set_world_map();
            MainInterfaceGlobal.button_group(1).set(0x3EE, 1);
            return;
        case 0x323:
            if (MapWinTable[0]) MapWinTable[0]->set_center(tile_x, tile_y, 1);
            return;
        case 0x324:
            set_center(tile_x, tile_y, 1);
            return;
        case 0x325:
            if (static_cast<int>(field_1DD98_) >= 0) return;
            field_1DD98_ += 2;
            on_redraw();
            return;
        case 0x321:
            if (static_cast<int>(field_1DD98_) <= -0xE) return;
            field_1DD98_ -= 2;
            on_redraw();
            return;
        case 0x322:
            if (static_cast<int>(field_1DD98_) == -8) return;
            field_1DD98_ = static_cast<uint32_t>(-8);
            on_redraw();
            return;
        default:
            return;
        }
    }

    if (ConsoleGlobal.cursor_active_ != 0 && vehicle_id >= 0 &&
        stack_check(vehicle_id, 1, -1, -1, -1) == 2) {
        mapwin_add_context_label(menu, 0xB18, 0x6E);
    }
    mapwin_add_context_label(menu, 0x8D4, 2);
    if ((GameDataStateStorage.fields_.game_state_ & 0x4000U) != 0 && vehicle_id >= 0) {
        mapwin_add_context_label(menu, 0xDFC, 0x29A);
        mapwin_add_context_label(menu, 0x104C, 0x29B);
    }
    if (base_id >= 0 && vehicle_id >= 0 &&
        stack_check(vehicle_id, 0xA, ConsoleGlobal.faction_id_, -1, -1) != 0 &&
        ConsoleGlobal.edit_lock() == 0) {
        mapwin_add_context_label(menu,
            stack_check(vehicle_id, 1, -1, -1, -1) == 1 ? 0x8E0 : 0x8E4,
            6);
    }
    menu.add_separator(0x63);

    if (base_id >= 0) {
        const Base &base = Bases[base_id];
        const int faction = ConsoleGlobal.faction_id_;
        const int owner = base.faction_id_current;
        if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) == 0 && owner != faction &&
            spying(owner) == 0 &&
            (PlayersData[faction].diplo_treaties[owner] & 1U) == 0) {
            mapwin_add_context_label(menu, 0x894, 5);
        }
        if ((GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0 ||
            base.faction_id_current == faction) {
            mapwin_add_parenthesized_label(menu, 0x898, 0x33);
            mapwin_add_base_production(menu, base, 0x34);
            mapwin_add_parenthesized_label(menu, 0x8A0, 0x35);
            mapwin_add_parenthesized_label(menu, 0x8A4, 0x36);
            mapwin_add_parenthesized_label(menu, 0x8A8, 0x37);
            mapwin_add_parenthesized_label(menu, 0x8AC, 0x38);
            if ((GameDataStateStorage.fields_.game_more_preferences_ & 4U) != 0) {
                mapwin_add_parenthesized_label(menu, 0x8B0, 0x39);
            }
        }
        menu.add_separator(0x63);
    }

    if (ConsoleGlobal.cursor_active_ != 0) {
        mapwin_add_context_label(menu, 0x8B4, 1);
        const int map_vehicle = ConsoleGlobal.selected_vehicle_;
        if (map_vehicle >= 0 &&
            map_vehicle < static_cast<int>(sizeof(Vehs) / sizeof(Vehs[0]))) {
            const Veh &vehicle = Vehs[map_vehicle];
            int delta_x = abs(tile_x - vehicle.x);
            if ((MapIsFlat & 1U) != 0 && delta_x > static_cast<int>(MapLongitude)) {
                delta_x = static_cast<int>(MapLongitudeBounds) - delta_x;
            }
            const int delta_y = abs(tile_y - vehicle.y);
            if (vehicle.x != tile_x || vehicle.y != tile_y) {
                if ((GameDataStateStorage.fields_.game_more_preferences_ & 4U) != 0) {
                    mapwin_add_context_label(menu, 0x8B8, 0x69);
                }
                const int prototype = vehicle.proto_id;
                if (prototype >= 0 && prototype < MaxVehProtoNum) {
                    const VehPrototype &unit = VehPrototypes[prototype];
                    if (unit.chassis_id >= 0 &&
                        unit.chassis_id < MaxChassisNum &&
                        Chassis[(int)unit.chassis_id].triad == TRIAD_AIR &&
                        unit.weapon_id < MaxWeaponNum &&
                        Weapon[unit.weapon_id].offense_rating != 0) {
                        mapwin_add_context_label(menu, 0x8BC, 0x6A);
                    }
                    if (has_abil(prototype, ABL_DROP_POD)) {
                        mapwin_add_context_label(menu, 0x8C0, 0x67);
                    }
                    if (can_arty(prototype, 1)) {
                        const int distance = (delta_y + delta_x) >> 1;
                        if (distance <= Rules.artillery_max_rng) {
                            mapwin_add_context_label(menu, 0x8C8, 0x68);
                        }
                    }
                }
            }
            if (((delta_y + delta_x) & ~1) <= 2) {
                mapwin_add_context_label(menu, 0x8CC, 0x65);
                if (stack_check(map_vehicle, 1, -1, -1, -1) == 9) {
                    mapwin_add_context_label(menu, 0x8D0, 0x66);
                }
            }
        }
        mapwin_add_context_label(menu, 0xE1C, 0xC7);
        mapwin_add_context_label(menu, 0x1098, 0xC8);
        menu.add_separator(0x63);
        if (ConsoleGlobal.edit_lock() != 0) {
            mapwin_add_context_label(menu, 0x8D8, 4);
        } else {
            mapwin_add_context_label(menu, 0x8DC, 3);
        }
    } else if (ConsoleGlobal.edit_lock() == 0) {
        mapwin_add_context_label(menu, 0x8DC, 3);
    }

    if (base_id < 0 && vehicle_id >= 0 &&
        stack_check(vehicle_id, 0xA, ConsoleGlobal.faction_id_, -1, -1) != 0 &&
        ConsoleGlobal.edit_lock() == 0) {
        mapwin_add_context_label(menu,
            stack_check(vehicle_id, 1, -1, -1, -1) == 1 ? 0x8E0 : 0x8E4,
            6);
    }
    mapwin_add_context_label(menu, 0x8E8, 8);
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 4U) != 0) {
        menu.add_separator(0x63);
        if (find_landmark(tile_x, tile_y, 1) >= 0) {
            mapwin_add_context_label(menu, 0x8EC, 9);
            mapwin_add_context_label(menu, 0x8F0, 0xB);
        } else {
            mapwin_add_context_label(menu, 0x8F4, 9);
        }
        menu.add_separator(0x63);
        mapwin_add_context_label(menu, 0x8F8, 7);
        mapwin_add_context_label(menu, 0x8FC, 0xA);
    }

    if (ConsoleControlTurnActive != 0) {
        menu.disable_item(1);
        menu.disable_item(3);
        menu.disable_item(4);
        menu.disable_item(5);
        menu.disable_item(8);
        menu.disable_item(9);
        menu.disable_item(0xB);
        int disabled_id;
        for (disabled_id = 0x33; disabled_id <= 0x39; ++disabled_id)
            menu.disable_item(disabled_id);
        for (disabled_id = 0x64; disabled_id <= 0x6E; ++disabled_id)
            menu.disable_item(disabled_id);
    }

    menu.show(screen_x, screen_y, pop_expire);
    if ((GameDataStateStorage.fields_.game_state_ & 0x800U) != 0) return;
    const int item_id = menu.get_selected_id();
    if (base_at(tile_x, tile_y) != base_id ||
        veh_at(tile_x, tile_y) != vehicle_id ||
        ConsoleGlobal.selected_vehicle_ != selected_vehicle) {
        return;
    }
    int edi = 0;
    int esi = 0;
    if (IsMultiplayerNet != 0) {
        edi = (ConsoleControlTurnActive == 0 &&
               NetDaemonNet.lock_state_[2].payload_.flags == 0 &&
               NetDaemonNet.lock_state_[0].leading_words_[3] == 0) ? 0 : 1;
        esi = (1 << ConsoleGlobal.faction_id_) &
              static_cast<int>(NetDaemonNet.lock_state_[0].trailing_words_[7]);
        base_id = base_at(tile_x, tile_y);
        vehicle_id = veh_at(tile_x, tile_y);
        selected_vehicle = ConsoleGlobal.selected_vehicle_;
    }
    ConsoleGlobal.get_mouse_pos(&screen_x, &screen_y);
    client_to_screen(&screen_x, &screen_y);

    switch (item_id) {
    case 1: {
        if (edi || esi) return;
        int locked_vehicle = selected_vehicle;
        if (locked_vehicle < 0 ||
            NetDaemonNet.lock_veh(&locked_vehicle, esi, -1, -1, esi) != 0) return;
        Vehs[locked_vehicle].order = ORDER_MOVE_TO;
        Vehs[locked_vehicle].waypoint_x[0] = static_cast<int16_t>(tile_x);
        Vehs[locked_vehicle].waypoint_y[0] = static_cast<int16_t>(tile_y);
        Vehs[locked_vehicle].state &= ~0x00010000U;
        synch_veh(locked_vehicle);
        NetDaemonNet.unlock_veh();
        return;
    }
    case 2:
        ConsoleGlobal.set_view(0);
        ConsoleGlobal.set_cursor(tile_x, tile_y);
        set_center(tile_x, tile_y, 1);
        return;
    case 3:
        if (edi || esi) return;
        ConsoleGlobal.set_move(esi);
        if (selected_vehicle >= 0) return;
        GameDataStateStorage.fields_.game_state_ |= 2U;
        ConsoleGlobal.field_23BE4_ = static_cast<uint32_t>(esi);
        return;
    case 4:
        if (selected_vehicle >= 0 &&
            selected_vehicle < static_cast<int>(sizeof(Vehs) / sizeof(Vehs[0]))) {
            const Veh &vehicle = Vehs[selected_vehicle];
            ConsoleGlobal.set_center(vehicle.x, vehicle.y, 1);
        }
        return;
    case 5:
        if (base_id >= 0) TutWinBaseWindow.zoom(base_id, 0);
        return;
    case 6:
        if (vehicle_id >= 0 &&
            stack_check(vehicle_id, 0xA, ConsoleGlobal.faction_id_, -1, -1) != 0) {
            ConsoleGlobal.activate(tile_x, tile_y, 0);
        }
        return;
    case 7:
        ConsoleStatusWin.set_loc(tile_x, tile_y);
        ConsoleStatusWin.redraw();
        ConsoleInfoWin.set_state(-1, 0);
        return;
    case 8:
        if (edi || esi) return;
        if ((GameDataStateStorage.fields_.game_state_ & 2U) == 0) {
            ConsoleGlobal.set_move(1);
            if (ConsoleGlobal.cursor_active_ != 0 && selected_vehicle >= 0) {
                if (X_pop("REALLYOVER", nullptr) == 0) return;
            }
        }
        GameDataStateStorage.fields_.game_state_ |= 2U;
        ConsoleGlobal.field_23BE4_ = static_cast<uint32_t>(esi);
        ConsoleGlobal.field_23BE8_ = static_cast<uint32_t>(esi);
        return;
    case 9:
        if (edi || esi) return;
        ConsoleGlobal.name_landmark(tile_x, tile_y);
        return;
    case 0xA:
        help_any();
        return;
    case 0xB:
        ConsoleGlobal.erase_landmark(tile_x, tile_y);
        return;
    case 0x33:
    case 0x65: {
        Menu &main_menu = ConsoleGlobal.alpha_menu_;
        const int index = main_menu.id_to_index(3);
        if (index >= 0 && index < 15 && main_menu.entries_[index].pull_down) {
            main_menu.entries_[index].pull_down->show(screen_x, screen_y, 0);
        }
        return;
    }
    case 0x34:
        if (edi || esi) return;
        if (base_id >= 0) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.hurry();
        }
        return;
    case 0x35:
    case 0x36:
    case 0x37:
    case 0x38:
        if (edi || esi) return;
        if (base_id >= 0) {
            TutWinBaseWindow.soft_zoom(base_id);
            TutWinBaseWindow.gov_on(0x01000000U << (item_id - 0x35));
        }
        return;
    case 0x39:
        if (edi || esi) return;
        if (base_id >= 0) ConsoleGlobal.go_to(base_id, -1, esi);
        return;
    case 0x66: {
        Menu &main_menu = ConsoleGlobal.alpha_menu_;
        const int index = main_menu.id_to_index(9);
        if (index >= 0 && index < 15 && main_menu.entries_[index].pull_down) {
            main_menu.entries_[index].pull_down->show(screen_x, screen_y, 0);
        }
        return;
    }
    case 0x67:
        if (edi || esi) return;
        veh_drop(selected_vehicle, tile_x, tile_y);
        return;
    case 0x68:
        if (edi || esi) return;
        action_arty(selected_vehicle, tile_x, tile_y);
        return;
    case 0x69:
        if (edi || esi) return;
        action_patrol(selected_vehicle, tile_x, tile_y);
        return;
    case 0x6A:
        if (edi || esi) return;
        ConsoleGlobal.set_bombing_run(selected_vehicle, tile_x, tile_y);
        return;
    case 0x6E:
        if (edi || esi) return;
        ConsoleGlobal.go_home(selected_vehicle);
        return;
    case 0xC7:
        if (edi || esi) return;
        ConsoleGlobal.destroy(selected_vehicle);
        return;
    case 0xC8:
        if (edi || esi) return;
        ConsoleGlobal.disband(selected_vehicle);
        return;
    case 0x29A:
        if (edi || esi) return;
        ConsoleGlobal.editor_edit_veh(vehicle_id);
        return;
    case 0x29B:
        if (edi || esi) return;
        ConsoleGlobal.editor_kill_veh(vehicle_id);
        return;
    default:
        return;
    }
}

// Original0x00469720 labels the eight neighboring tiles around the current
// console cursor using the original keypad order and this window's map scale.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00469720 ?draw_legend@MapWin@@QAEXXZ 0x00469720-0x004698C8;0x00655932-0x00655944 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00469720.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void MapWin::draw_legend() {
    Font legend_font;
    legend_font.init(DefaultFontFace,
                     (static_cast<int>(field_1DDB8_) * 2) / 3, 1);
    Buffer *const canvas = static_cast<Buffer *>(this);
    const int cursor = ConsoleGlobal.cursor_history_index_;

    for (int direction = 0; direction < 8; ++direction) {
        int tile_x = ConsoleGlobal.cursor_history_x_[cursor] +
                     VehMoveDeltaX[direction];
        const int tile_y = ConsoleGlobal.cursor_history_y_[cursor] +
                           VehMoveDeltaY[direction];
        if ((MapIsFlat & 1) == 0) {
            if (tile_x < 0) {
                tile_x += MapLongitudeBounds;
            } else if (tile_x >= MapLongitudeBounds) {
                tile_x -= MapLongitudeBounds;
            }
        }
        if (tile_y < 0 || tile_y >= MapLatitudeBounds || tile_x < 0 ||
            tile_x >= MapLongitudeBounds) {
            continue;
        }
        int pixel_x;
        int pixel_y;
        tile_to_pixel(tile_x, tile_y, &pixel_x, &pixel_y);
        _itoa(MapDirectionPermutation[direction], StringTemp, 10);
        canvas->set_text_color(0xFF, 0, 1, 1);
        canvas->write_cent_l(&legend_font, StringTemp, pixel_x, pixel_y,
                             static_cast<int>(field_1DDB4_),
                             static_cast<int>(field_1DDB8_),
                             static_cast<int>(strlen(StringTemp)));
    }
}

// Original0x004698D0 draws the seven-by-seven diplomacy key to the right of
// the map. The row and column indices both omit faction zero, the native-life
// record, exactly as the original table bounds do.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x004698D0 ?draw_wars@MapWin@@QAEXXZ 0x004698D0-0x00469B32 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/004698d0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void MapWin::draw_wars() {
    Font *const font = &font_b_;
    const int character_width = font->width("FUNGUS");
    const int line_advance = font->line_advance();
    int y = line_advance;
    Buffer *const canvas = static_cast<Buffer *>(this);
    canvas->set_font(font, 0, 0, 0);
    int max_width = character_width;

    for (int faction = 1; faction < MaxPlayerNum; ++faction) {
        GenderDefault = Players[faction].noun_gender;
        PluralityDefault = Players[faction].is_noun_plural;
        StringTemp[0] = '\0';
        strcat(StringTemp, Players[faction].noun_faction);
        strcat(StringTemp, ": ");
        canvas->set_text_color(InfoFactionTextColors[faction], 0, 1, 1);
        const int width = canvas->write_l(
            StringTemp, character_width, y, strlen(StringTemp));
        if (max_width <= width) {
            max_width = width;
        }
        y += line_advance;
    }

    char letter = '-';
    y = line_advance;
    for (int row = 1; row < MaxPlayerNum; ++row) {
        int x = max_width;
        for (int column = 1; column < MaxPlayerNum; ++column) {
            canvas->set_text_color(InfoFactionTextColors[column], 0, 1, 1);
            const uint32_t flags = PlayersData[row].diplo_treaties[column];
            if ((flags & 8U) == 0) {
                letter = '-';
            } else if ((flags & 0x10U) != 0) {
                letter = 'W';
            } else if ((flags & 1U) != 0) {
                letter = 'P';
            } else if ((flags & 2U) != 0) {
                letter = 'T';
            } else if ((flags & 4U) != 0) {
                letter = 't';
            }
            StringTemp[0] = letter;
            StringTemp[1] = '\0';
            canvas->write_l(StringTemp, x, y, strlen(StringTemp));
            x += character_width;
        }
        y += line_advance;
    }
}

// Original0x0046D5D0 handles a map click as one Popup lifetime spanning the
// editor, base-routing, cursor-order and map-selection paths. The local
// cursor mode is captured before Console::selected clears it; this is also
// the value consumed by the order dispatch near the end of the original.
// ORIGINAL: 0x0046D5D0 ?click@MapWin@@QAEXHHH@Z 0x0046D5D0-0x0046EB68;0x00655959-0x00655E92 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/0046d5d0.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// TRIED: GENUINELY PARTIAL - literal register-machine transliteration (same technique as 0x00547C10/0x00448D90: raw EBP(n)-mapped buffer standing in for the Popup/Scroll/2xFlatButton ensemble, original_method<> calls for their lifecycle, goto per basic block) but covers roughly the first ~1/4 of the function only (asm 0x0046D5D0-0x0046DD86 of a 0x46D5D0-0x46EB38+ span: the SEH-free prologue, the AlphaMenu/InfoWin/StatusWin/TutWin reset calls, the editor-mode/key-state checks, Console::edit_lock gate, pixel_to_tile click resolution, the world-climate redraw dispatch, and the NetDaemon base-lock + Dialogs base-picker + port_to_port confirmation flow, each with its own full Popup-cleanup early-exit). Every remaining branch (unit orders, right-click menu, build/terraform dispatch, and the shared tail at 0x46EB38) is UNWRITTEN - the two forward labels it would reach (LAB_0046eac4, LAB_0046eb38) are stubbed empty and the function returns there instead. Left this way rather than guessing the remaining ~3/4: every call/branch actually present above is a faithful, push-order- verified transcription, not invented filler. 1643 rebuilt bytes against 5528 scored original bytes.
void MapWin::click(int pixel_x, int pixel_y, int button) {
    Popup popup;
    if (field_1DD88_ != 0) {
        field_1DD88_ = 0;
        return;
    }

    ConsoleGlobal.alpha_menu_.hide();
    ConsoleInfoWin.reset();
    ConsoleStatusWin.reset();
    TutorialWindow.reset();
    ConsoleGlobal.survey_mode_ = 0;
    TutWinBaseWindow.cancel_zoom_wait();

    const bool cursor_sprite_1 = cursor_sprite_ == &g_CURSOR_SPRITES[1];
    const bool cursor_sprite_9 = cursor_sprite_ == &g_CURSOR_SPRITES[9];
    const int editor_tool = field_1DE24_;
    const int editor_mode = field_1DD84_;
    go_reset();

    bool control_down = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0 &&
        (GetKeyState(VK_SCROLL) & 1) != 0 &&
        (GetAsyncKeyState(VK_SHIFT) & 0x8000) == 0) {
        control_down = !control_down;
    }

    const bool cursor_sprite_5 = cursor_sprite_ == &g_CURSOR_SPRITES[5];
    const int cursor_mode = ConsoleGlobal.selected();
    if (cursor_sprite_5 && cursor_mode != 0) return;
    if ((button != 0 || ConsoleControlTurnActive != 0) &&
        (cursor_sprite_1 || cursor_sprite_9 || cursor_mode != 0)) {
        return;
    }

    int tile_x;
    int tile_y;
    if (pixel_to_tile(pixel_x, pixel_y, &tile_x, &tile_y) != 0) return;
    if (tile_x < 0 || tile_x >= static_cast<int>(MapLongitudeBounds) ||
        tile_y < 0 || tile_y >= static_cast<int>(MapLatitudeBounds)) {
        return;
    }

    if (field_1DD8C_ != 0) {
        editor(tile_x, tile_y, static_cast<int>(field_1DD8C_ & 2));
        field_1DD8C_ = 0;
        if (MapEditorPaintId == 0 || MapEditorPaintId == 2 ||
            MapEditorPaintId == 0x12) {
            world_climate();
            for (size_t slot = 0; slot < MapWinTableSlots; ++slot) {
                MapWin *const window = MapWinTable[slot];
                if (window != nullptr &&
                    (slot == 0 || window->field_1DD74_ != 0)) {
                    window->draw_map(1);
                }
            }
        } else {
            ::draw_map(1);
        }
        return;
    }

    // The selected-base route is available only to the base cursor on the
    // owning faction. Base and triad records remain their actual source types.
    if (cursor_sprite_9 && editor_tool >= 0) {
        const Base &source = Bases[editor_tool];
        const int local_faction = ConsoleGlobal.faction_id_;
        if (source.faction_id_current == local_faction) {
            set_base(editor_tool);
            if (NetDaemonNet.lock_base(BaseIDCurrentSelected, 0, -1, -1) != 0) {
                return;
            }

            const int target_base = base_find(tile_x, tile_y, local_faction,
                                              -1, -2, -1);
            if (target_base < 0) {
                NetDaemonNet.unlock_base(BaseIDCurrentSelected);
                return;
            }
            parse_says(0, source.name_string, -1, -1);
            parse_says(1, Bases[target_base].name_string, -1, -1);
            popup.start(PopupStartCaption, "DESTTRIAD2", -1, nullptr,
                        0x40, nullptr);
            for (int triad = 0; triad < MaxTriadNum; ++triad) {
                StringTemp[0] = '\0';
                strcat(StringTemp, StringTable.get(
                    reinterpret_cast<int>(Triad[triad])));
                strcat(StringTemp, " ");
                strcat(StringTemp, "(");
                const int destination = triad == 0
                    ? source.auto_forward_land_base_id
                    : triad == 1 ? source.auto_forward_sea_base_id
                                 : source.auto_forward_air_base_id;
                say_base(StringTemp, destination);
                strcat(StringTemp, ")");
                popup.dialogs().item(StringTemp, triad);
            }
            const int selected_triad = popup.exec(0, nullptr);
            if (selected_triad >= 0 && selected_triad < MaxTriadNum) {
                int destination = target_base;
                bool valid = true;
                if (target_base == editor_tool) {
                    destination = -1;
                } else if (selected_triad == 1) {
                    valid = port_to_port(target_base, editor_tool) != 0;
                } else if (selected_triad == 0) {
                    const Base &target = Bases[target_base];
                    const int source_offset = (source.x >> 1) +
                        static_cast<int>(source.y) * static_cast<int>(MapLongitude);
                    const int target_offset = (target.x >> 1) +
                        static_cast<int>(target.y) * static_cast<int>(MapLongitude);
                    valid = map_tile_bytes()[source_offset * 4 + 3] ==
                            map_tile_bytes()[target_offset * 4 + 3];
                }
                if (valid) {
                    int16_t *route = selected_triad == 0
                        ? &Bases[editor_tool].auto_forward_land_base_id
                        : selected_triad == 1
                            ? &Bases[editor_tool].auto_forward_sea_base_id
                            : &Bases[editor_tool].auto_forward_air_base_id;
                    *route = static_cast<int16_t>(destination);
                } else {
                    parse_say(2, reinterpret_cast<int>(Triad[selected_triad]),
                              -1, -1);
                    NetMsgPrimary.pop("DESTNO", 0x1388, 0, nullptr);
                }
            }
            NetDaemonNet.unlock_base(BaseIDCurrentSelected);
            return;
        }
    }

    // Cursor sprite 1 applies a fake group-order marker or edits waypoints.
    // Other cursor sprites go directly to their separate action dispatcher.
    if (cursor_sprite_1) {
        int vehicle_id = ConsoleGlobal.selected_vehicle_;
        if ((ConsoleGlobal.cursor_active_ == 0 || vehicle_id < 0) &&
            ConsoleGlobal.field_23D1C_ != 0) {
            ConsoleGlobal.set_cursor(tile_x, tile_y);
            vehicle_id = veh_fake(0, ConsoleGlobal.faction_id_);
            Veh &fake_vehicle = Vehs[vehicle_id];
            fake_vehicle.x = static_cast<int16_t>(tile_x);
            fake_vehicle.y = static_cast<int16_t>(tile_y);
            fake_vehicle.order = 0x18;
            fake_vehicle.waypoint_x[0] = static_cast<int16_t>(tile_x);
            fake_vehicle.waypoint_y[0] = static_cast<int16_t>(tile_y);
            fake_vehicle.state = 0;
            ConsoleGlobal.group(vehicle_id);
        } else if (vehicle_id >= 0) {
            int locked_vehicle = vehicle_id;
            if (NetDaemonNet.lock_veh(&locked_vehicle,
                    editor_mode != 0 ? 4 : 0, -1, -1, 0) == 0) {
                vehicle_id = locked_vehicle;
                Veh &vehicle = Vehs[vehicle_id];
                if ((vehicle.state & 0x02000000U) != 0) {
                    const int waypoint = vehicle.waypoint_count;
                    vehicle.waypoint_x[waypoint] = static_cast<int16_t>(tile_x);
                    vehicle.waypoint_y[waypoint] = static_cast<int16_t>(tile_y);
                    vehicle.state |= 0x02000200U;
                    vehicle.state &= ~0x00010000U;
                } else {
                    vehicle.waypoint_x[0] = static_cast<int16_t>(tile_x);
                    vehicle.waypoint_y[0] = static_cast<int16_t>(tile_y);
                    vehicle.state &= ~0x02010000U;
                    vehicle.waypoint_count = 0;
                    vehicle.patrol_current_point = 0;
                }
                if (editor_mode != 0) ConsoleGlobal.group(vehicle_id);
                synch_veh(vehicle_id);
                NetDaemonNet.unlock_veh();
                if (ConsoleGlobal.cursor_active_ == 0) ConsoleGlobal.set_move(1);
            }
        }
        return;
    }

    if (cursor_mode != 0 && ConsoleGlobal.selected_vehicle_ >= 0 &&
        cursor_mode >= 2 && cursor_mode <= 11) {
        switch (cursor_mode) {
        case 2:
            ConsoleGlobal.set_road_to(ConsoleGlobal.selected_vehicle_,
                                      tile_x, tile_y, 0x1B);
            break;
        case 3:
            ConsoleGlobal.set_road_to(ConsoleGlobal.selected_vehicle_,
                                      tile_x, tile_y, 0x1C);
            break;
        case 4:
            // This is NetDaemon::veh_drop (0x00532140), not the unrelated
            // free vehicle-list helper at 0x005C0080. Its source declaration
            // is the source-owned network operation, with its lock protocol.
            NetDaemonNet.veh_drop(ConsoleGlobal.selected_vehicle_,
                                  tile_x, tile_y);
            break;
        case 8:
            action_patrol(ConsoleGlobal.selected_vehicle_, tile_x, tile_y);
            break;
        case 10:
            action_arty(ConsoleGlobal.selected_vehicle_, tile_x, tile_y);
            break;
        case 11:
            ConsoleGlobal.set_bombing_run(ConsoleGlobal.selected_vehicle_,
                                          tile_x, tile_y);
            break;
        default:
            break;
        }
        ConsoleGlobal.field_23BE4_ = 0;
        return;
    }

    if ((GameDataStateStorage.fields_.game_state_ & STATE_SCENARIO_EDITOR) != 0 && control_down &&
        ConsoleControlTurnActive == 0) {
        ConsoleGlobal.set_view(1);
        ConsoleGlobal.set_cursor(tile_x, tile_y);
        auto_undo();
        editor(tile_x, tile_y, button);
        return;
    }

    if (button == 0) {
        const bool shift_down = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
        if (!shift_down && control_down) {
            ConsoleStatusWin.set_loc(tile_x, tile_y);
            ConsoleStatusWin.redraw();
            ConsoleInfoWin.set_state(-1, 0);
            return;
        }
        if (!shift_down && !control_down) {
            if ((GameDataStateStorage.fields_.game_state_ & STATE_UNK_100) != 0 ||
                ConsoleControlTurnActive != 0) {
                ConsoleGlobal.set_view(button);
                ConsoleGlobal.set_cursor(tile_x, tile_y);
                const bool primary = this == MapWinTable[0];
                if (primary) field_21A44_ = 1;
                set_center(tile_x, tile_y, 1);
                if (primary) field_21A44_ = 0;
            } else {
                right_menu(tile_x, tile_y, pixel_x, pixel_y);
            }
            return;
        }
        if (((GameDataStateStorage.fields_.game_state_ & STATE_UNK_100) != 0 || field_1DD78_ == 1) &&
            ConsoleControlTurnActive == 0) {
            right_menu(tile_x, tile_y, pixel_x, pixel_y);
            return;
        }
    }

    if (field_1DD78_ == 0) {
        const bool skip_cursor_update = ConsoleGlobal.cursor_active_ == 0 &&
            ConsoleGlobal.selected_vehicle_ == 0 && button == 0 &&
            ConsoleControlTurnActive == 0 && ConsoleGlobal.edit_lock() == 0;
        if (!skip_cursor_update) {
            ConsoleGlobal.set_view(button);
            ConsoleGlobal.set_cursor(tile_x, tile_y);
        }
    }
    if (button == 0 && static_cast<int32_t>(field_1DD7C_) > 1) return;

    const int base_id = base_at(tile_x, tile_y);
    const int local_faction = ConsoleGlobal.faction_id_;
    if (field_1DD78_ != 1 && base_id >= 0 && button == 0 &&
        ConsoleControlTurnActive == 0) {
        const Base &base = Bases[base_id];
        const int owner = base.faction_id_current;
        const bool omniscient = (GameDataStateStorage.fields_.game_state_ & STATE_OMNISCIENT_VIEW) != 0;
        const bool visible = owner == local_faction ||
            (base.visibility & (1U << local_faction)) != 0;
        const bool access = omniscient || owner == local_faction ||
            spying(owner) != 0 ||
            (PlayersData[local_faction].diplo_treaties[owner] & 1U) != 0;
        if ((visible || omniscient) && access) {
            set_center(tile_x, tile_y, 0);
            if (field_1DD78_ != 0 && (field_1DD70_ & 0x40000000U) != 0)
                MapWinTable[0]->set_center(tile_x, tile_y, 0);
            TutWinBaseWindow.zoom(base_id, 0);
            return;
        }
    }

    const bool primary = this == MapWinTable[0];
    if (primary) field_21A44_ = 1;
    set_center(tile_x, tile_y, 1);
    if (primary) field_21A44_ = 0;
    const int vehicle_id = veh_at(tile_x, tile_y);
    if (field_1DD78_ != 1 && vehicle_id >= 0 &&
        Vehs[vehicle_id].faction_id == local_faction && button == 0 &&
        ConsoleControlTurnActive == 0 && ConsoleGlobal.edit_lock() == 0) {
        ConsoleGlobal.activate(tile_x, tile_y, 0);
    }
}

// Original0x00467970 paints visible base destinations and the local faction's
// vehicle waypoint/transport routes over the map. Both record families are
// owned arrays in base.h and veh.h; no archive-shaped mirror is needed.
// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x00467970 ?draw_base_dest@MapWin@@QAEXH@Z 0x00467970-0x00467DD9 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/00467970.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void MapWin::draw_base_dest(int draw_type) {
    bool any = false;
    bool drew_base_line = false;
    const int local_faction = ConsoleGlobal.faction_id_;
    const int visible_bottom = mapwin_dest_add(
        field_1DDA8_, mapwin_dest_add(field_1DDD8_, field_1DDD0_));
    const int visible_right = mapwin_dest_add(
        field_1DDA4_, mapwin_dest_add(field_1DDD4_, field_1DDCC_));

    if ((field_1DD70_ & 0x40000U) != 0) {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; ++base_id) {
            const Base &base = Bases[base_id];
            if (base.faction_id_current != local_faction) {
                continue;
            }
            const int destinations[3] = {
                base.auto_forward_land_base_id,
                base.auto_forward_sea_base_id,
                base.auto_forward_air_base_id
            };
            for (int direction = 0; direction < 3; ++direction) {
                const int destination_id = destinations[direction];
                if (destination_id < 0) {
                    continue;
                }
                const Base &destination = Bases[destination_id];
                const bool source_row_visible =
                    base.y >= field_1DDA8_ && base.y < visible_bottom;
                bool line_visible = false;
                if (source_row_visible) {
                    if (destination.y >= field_1DDA8_ &&
                        destination.y < visible_bottom) {
                        int destination_x = destination.x;
                        if ((MapIsFlat & 1) == 0) {
                            if (destination_x < field_1DDA4_) {
                                destination_x = mapwin_dest_add(
                                    destination_x, MapLongitudeBounds);
                            }
                            if (destination_x >= visible_right) {
                                destination_x = mapwin_dest_sub(
                                    destination_x, MapLongitudeBounds);
                            }
                        }
                        line_visible = destination_x >= field_1DDA4_ &&
                                       destination_x < visible_right;
                    }
                } else {
                    int source_x = base.x;
                    if ((MapIsFlat & 1) == 0) {
                        if (source_x < field_1DDA4_) {
                            source_x = mapwin_dest_add(source_x,
                                                    MapLongitudeBounds);
                        }
                        if (source_x >= visible_right) {
                            source_x = mapwin_dest_sub(source_x,
                                                    MapLongitudeBounds);
                        }
                    }
                    line_visible = source_x >= field_1DDA4_ &&
                                   source_x < visible_right;
                }
                if (!line_visible) {
                    continue;
                }
                const int line_type = direction == 0 || direction == 1 ||
                                      direction == 2 ? 10 : 0;
                dest_line(base.x, base.y, destination.x, destination.y,
                          line_type, local_faction, direction, 0, 3, 5, 0);
                drew_base_line = true;
            }
        }
    }

    if ((field_1DD70_ & 0xC00000U) != 0) {
        for (int vehicle_id = 0; vehicle_id < GameDataStateStorage.fields_.vehicle_current_count_; ++vehicle_id) {
            const Veh &vehicle = Vehs[vehicle_id];
            if (vehicle.faction_id != local_faction) {
                continue;
            }
            const uint32_t relevant_state = vehicle.state & 0x01000200U;
            const bool repeated_route = relevant_state == 0x01000200U &&
                                        (field_1DD70_ & 0x400000U) != 0;
            const bool ordered_route = vehicle.order == ORDER_MOVE_TO &&
                                       (field_1DD70_ & 0x800000U) != 0;
            if (!repeated_route && !ordered_route) {
                continue;
            }

            const int prototype = vehicle.proto_id;
            const int triad = get_proto_triad(prototype);
            const int cover = relevant_state == 0x01000200U ? 1 : 0;
            if (repeated_route) {
                const int count = vehicle.waypoint_count;
                const int segment_count = count + (count > 1 ? 1 : 0);
                for (int segment = 0; segment < segment_count; ++segment) {
                    const int start = segment % (count + 1);
                    const int finish = (segment + 1) % (count + 1);
                    dest_line(vehicle.waypoint_x[start],
                              vehicle.waypoint_y[start],
                              vehicle.waypoint_x[finish],
                              vehicle.waypoint_y[finish], prototype,
                              vehicle.faction_id, triad, cover, 3, 5, 0);
                }
            } else {
                dest_line(vehicle.x, vehicle.y, vehicle.waypoint_x[0],
                          vehicle.waypoint_y[0], prototype,
                          vehicle.faction_id, triad, cover, 2, 2, 0);
                if ((vehicle.state & VSTATE_UNK_2000000) != 0 &&
                    vehicle.waypoint_count != 0) {
                    for (int point = 0; point < vehicle.waypoint_count;
                         ++point) {
                        dest_line(vehicle.waypoint_x[point],
                                  vehicle.waypoint_y[point],
                                  vehicle.waypoint_x[point + 1],
                                  vehicle.waypoint_y[point + 1], prototype,
                                  vehicle.faction_id, triad, cover, 2, 2, 0);
                    }
                }
            }
            any = true;
        }
    }

    if (draw_type != 0 && (drew_base_line || any)) {
        on_update();
    }
}

// ORIGINAL: 0x00468CD0 ?compute_clip@MapWin@@QAEHHHHPAURECT@@@Z 0x00468CD0-0x00468F8D FILE
// symbol    ?compute_clip@MapWin@@QAEHHHHPAUtagRECT@@@Z
//           The same windows.h `typedef struct tagRECT` mangling every other
//           `RECT *` parameter in this tree carries; the bytes are the body below.
// TRIED: this->field_1DDC0_ (offset 0x1DDC0, matches self[0x7770] and the class's own field name) and the virtual-base offset table at *(int*)this (vbptr[1] gives the displacement, then +this+0x474 or +0x4c8) reproduce the two IntersectRect-source-rect and default-bottom reads; diverges early (#6) given the function's size (701 bytes, four scan loops) - not chased further.
// working copy - scaffold materialised by --work
// size      701 bytes
// prototype int (__thiscall ?compute_clip@MapWin@@QAEHHHHPAURECT@@@Z)(MapWin* this, int, int, int, RECT*)
// callers   3   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462F00
// indirect  0x00468EF0 0x00468F58


int MapWin::compute_clip(int a1, int a2, int a3, RECT * a4) {
    int *self = reinterpret_cast<int *>(this);
    RECT rect;
    int px, py;
    int count;
    int row;

    // LEFT edge
    int leftCol = a1 - a3 * 2;
    rect.left = 100000;
    if ((MapIsFlat & 1) == 0) {
        if (leftCol < 0) {
            leftCol = MapLongitudeBounds + leftCol;
        } else if (MapLongitudeBounds <= leftCol) {
            leftCol = leftCol - MapLongitudeBounds;
        }
    }
    if (a3 >= 0) {
        row = a2 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            if (row >= 0 && row < MapLatitudeBounds && leftCol >= 0 && leftCol < MapLongitudeBounds) {
                tile_to_pixel(leftCol, row, &px, &py);
                if (px < rect.left) {
                    rect.left = px;
                }
            }
            row += 2;
            count--;
        } while (count != 0);
    }

    // RIGHT edge
    rect.right = 0;
    int rightCol = a1 + 2 + a3 * 2;
    if ((MapIsFlat & 1) == 0) {
        if (rightCol < 0) {
            rightCol = MapLongitudeBounds + rightCol;
        } else if (MapLongitudeBounds <= rightCol) {
            rightCol = rightCol - MapLongitudeBounds;
        }
    }
    if (a3 >= 0) {
        row = a2 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            if (row >= 0 && row < MapLatitudeBounds && rightCol >= 0 && rightCol < MapLongitudeBounds) {
                tile_to_pixel(rightCol, row, &px, &py);
                if (rect.right < px) {
                    rect.right = px;
                }
            }
            row += 2;
            count--;
        } while (count != 0);
    }

    // TOP edge
    int topRow = a2 - a3 * 2;
    rect.top = 100000;
    if (topRow < 0 || MapLatitudeBounds <= topRow || a1 < 0 || MapLongitudeBounds <= a1) {
        rect.top = 0;
    } else if (a3 >= 0) {
        int col = a1 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            tile_to_pixel(col, topRow, &px, &py);
            if (py < rect.top) {
                rect.top = py;
            }
            col += 2;
            count--;
        } while (count != 0);
    }

    // BOTTOM edge
    rect.bottom = 0;
    int bottomRow = a2 + 2 + a3 * 2;
    if (bottomRow < 0 || MapLatitudeBounds <= bottomRow || a1 < 0 || MapLongitudeBounds <= a1) {
        rect.bottom = -*reinterpret_cast<int *>(*reinterpret_cast<int *>(*self + 4) + 0x4c8 + reinterpret_cast<int>(self));
    } else if (a3 >= 0) {
        int col = a1 - a3 * 2;
        count = a3 * 2 + 1;
        do {
            tile_to_pixel(col, bottomRow, &px, &py);
            if (rect.bottom < py) {
                rect.bottom = py;
            }
            col += 2;
            count--;
        } while (count != 0);
    }

    rect.top = rect.top - self[0x7770];

    int hit = IntersectRect(a4, &rect, &rect2_);
    if (hit != 0) {
        if (a3 == 0) {
            int vgrow = (rect.bottom - rect.top) / 4;
            int hgrow = (rect.right - rect.left) / 4;
            rect.left = rect.left - hgrow;
            rect.right = rect.right + hgrow;
            rect.top = rect.top - vgrow;
            rect.bottom = rect.bottom + vgrow;
        }
        IntersectRect(a4, &rect, &rect2_);
    }

    if (a4->right != a4->left && a4->right - a4->left >= 0 && a4->bottom - a4->top > 0) {
        return 1;
    }
    return 0;
}

/*
Purpose: Generate the whole visible map - background, terrain, vehicles,
         overlays, base names, legend and territory wars. Three row-major
         scans walk the same window rectangle: rows come from field_1DDA8_
         plus the scan index, columns start at field_1DDA4_ biased by the
         ROW's parity - odd rows sit one tile right and take their column
         count from field_1DDCC_ where even rows take it from field_1DDD4_ -
         and the column steps two tiles at a time because SMAC rows
         interleave.
         Each visited column is seam-wrapped once against MapLongitudeBounds
         unless the map is flat, then bounds-gated, then optionally gated by
         the two zoom rectangles at horizontal_damage_.left and vertical_damage_.left - whose
         own left/right order may be swapped on a cylindrical map, in which
         case membership is the union of the two arcs. The vehicle and
         overlay scans admit a two-tile margin around those rectangles.
         The overlay scan still calls tile_to_pixel even though it discards
         the pixel pair: the image does the same (0x0046A215), and the call
         cannot be dropped without changing behavior visible to a replacing
         callee.
// ORIGINAL: 0x00469CA0 ?gen_map@MapWin@@QAEXHH@Z 0x00469CA0-0x0046A292 FILE
// RECOVERY_BINDING: v1 {"evidence": ["Original469CA0..46A292; retired seed git:b55fcf8b:src/unrecovered/00469ca0.cpp records this identity and exact span."], "reason": "The three scan passes are implemented as a typed MapWin member over its owned viewport and damage rectangles, with the image's terrain, vehicle, overlay, label and legend helpers retained as real calls."}
// TRIED (retired seed git:b55fcf8b:src/unrecovered/00469ca0.cpp): faithful three-loop transcription matched through instruction #28 and diverged at
//            #29 on the i&1 parity scheduling in the row/column base
//            computation; hoisting `i & 1` to a temp changed nothing. The
//            body below spells the scans idiomatically instead.
// TRIED: 2026-09-21 variant deck (four spellings of the row/col base and
//            count select: inline ternary, hoisted parity temp, parity
//            between the two bases, if-selected count) - all four compile
//            gen_map to identical sizes (1488 bytes /c /O2 /Gy /GR- /GX,
//            1456 bytes with /Oy-) against the original's 1522, so the
//            parity spelling is byte-neutral and the ternary form stays.
//            The remaining extent gap is allocator-shaped: the image
//            memory-homes the vehicle/overlay scans' eight +/-2 zoom
//            margins at [ebp-0x30..-0x1c] through store-then-reload chains
//            inside a 0x38-byte frame, keeps the scan index in the dead a2
//            parameter slot and the third scan's column base in the dead a1
//            slot, and reloads MapLongitudeBounds once per iteration into a
//            register this tree re-reads; none of that is reachable by
//            spelling, and the emitted call/reference set (fill, both
//            bounds globals, MapIsFlat, tile_to_pixel, gen_terrain_poly,
//            draw_vehicles, gen_overlays, draw_base_names, draw_legend,
//            draw_wars, on_map) agrees with the image's nine targets.
// size      1522 bytes
// prototype void (__thiscall ?gen_map@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   1   call targets   9
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00462F00 0x004632D0 0x00466CA0 0x00467DE0 0x004688E0 0x00469720 0x004698D0 0x004712A0 0x005D5250
*/
void MapWin::gen_map(int a1, int a2) {
    if (a2 == 0) {
        if ((field_1DD70_ & 0x82000000) == 0 && field_21A44_ == 0) {
            GraphicWin::fill(0x41);
        }
        for (int i = 0; i < field_1DDD0_ + field_1DDD8_ + 1; i++) {
            const int row = field_1DDA8_ + i;
            int col = field_1DDA4_ + (i & 1);
            const int count = (i & 1) ? field_1DDCC_ : field_1DDD4_;
            for (int n = count; n > 0; n--, col += 2) {
                const int flat = MapIsFlat & 1;
                int wrapped = col;
                if (flat == 0) {
                    if (wrapped < 0) {
                        wrapped += MapLongitudeBounds;
                    } else if (wrapped >= MapLongitudeBounds) {
                        wrapped -= MapLongitudeBounds;
                    }
                }
                if (row >= 0 && row < MapLatitudeBounds &&
                    wrapped >= 0 && wrapped < MapLongitudeBounds &&
                    (field_21A44_ == 0 ||
                     (row >= (int)vertical_damage_.top && row <= (int)vertical_damage_.bottom &&
                      ((int)vertical_damage_.left > (int)vertical_damage_.right && flat == 0
                           ? wrapped >= (int)vertical_damage_.left ||
                                 wrapped < (int)vertical_damage_.right
                           : wrapped >= (int)vertical_damage_.left &&
                                 wrapped < (int)vertical_damage_.right)) ||
                     (row >= (int)horizontal_damage_.top && row <= (int)horizontal_damage_.bottom &&
                      ((int)horizontal_damage_.left > (int)horizontal_damage_.right && flat == 0
                           ? wrapped >= (int)horizontal_damage_.left ||
                                 wrapped < (int)horizontal_damage_.right
                           : wrapped >= (int)horizontal_damage_.left &&
                                 wrapped < (int)horizontal_damage_.right)))) {
                    int px;
                    int py;
                    tile_to_pixel(wrapped, row, &px, &py);
                    Buffer *const surface = this;
                    gen_terrain_poly(surface, this, field_1DD98_, a1, -1,
                                     field_1DD70_, wrapped, row, px, py);
                    on_map(wrapped, row);
                }
            }
        }
    }

    for (int i = 0; i < field_1DDD0_ + field_1DDD8_ + 1; i++) {
        const int row = field_1DDA8_ + i;
        int col = field_1DDA4_ + (i & 1);
        const int count = (i & 1) ? field_1DDCC_ : field_1DDD4_;
        for (int n = count; n > 0; n--, col += 2) {
            const int map_width = MapLongitudeBounds;
            const int flat = MapIsFlat & 1;
            int wrapped = col;
            if (flat == 0) {
                if (wrapped < 0) {
                    wrapped += map_width;
                } else if (wrapped >= map_width) {
                    wrapped -= map_width;
                }
            }
            if (row >= 0 && row < MapLatitudeBounds &&
                wrapped >= 0 && wrapped < map_width) {
                bool generate = field_21A44_ == 0;
                if (!generate) {
                    const int left1 = vertical_damage_.left - 2;
                    const int top1 = vertical_damage_.top - 2;
                    const int right1 = vertical_damage_.right + 2;
                    const int bottom1 = vertical_damage_.bottom + 2;
                    const int left2 = horizontal_damage_.left - 2;
                    const int top2 = horizontal_damage_.top - 2;
                    const int right2 = horizontal_damage_.right + 2;
                    const int bottom2 = horizontal_damage_.bottom + 2;
                    generate =
                        (row >= top1 && row <= bottom1 &&
                         (left1 > right1 && flat == 0
                              ? wrapped >= left1 || wrapped < right1
                              : wrapped >= left1 && wrapped < right1)) ||
                        (row >= top2 && row <= bottom2 &&
                         (left2 > right2 && flat == 0
                              ? wrapped >= left2 || wrapped < right2
                              : wrapped >= left2 && wrapped < right2));
                }
                if (generate) {
                    int px;
                    int py;
                    tile_to_pixel(wrapped, row, &px, &py);
                    draw_vehicles(wrapped, row, px, py - field_1DDC0_, a1);
                }
            }
        }
    }

    draw_base_names();

    if ((field_1DD70_ & 0x2000) != 0) {
        for (int i = 0; i < field_1DDD0_ + field_1DDD8_ + 1; i++) {
            const int row = field_1DDA8_ + i;
            int col = field_1DDA4_ + (i & 1);
            const int count = (i & 1) ? field_1DDCC_ : field_1DDD4_;
            for (int n = count; n > 0; n--, col += 2) {
                const int map_width = MapLongitudeBounds;
                const int flat = MapIsFlat & 1;
                int wrapped = col;
                if (flat == 0) {
                    if (wrapped < 0) {
                        wrapped += map_width;
                    } else if (wrapped >= map_width) {
                        wrapped -= map_width;
                    }
                }
                if (row >= 0 && row < MapLatitudeBounds &&
                    wrapped >= 0 && wrapped < map_width) {
                    bool generate = field_21A44_ == 0;
                    if (!generate) {
                        const int left1 = vertical_damage_.left - 2;
                        const int top1 = vertical_damage_.top - 2;
                        const int right1 = vertical_damage_.right + 2;
                        const int bottom1 = vertical_damage_.bottom + 2;
                        const int left2 = horizontal_damage_.left - 2;
                        const int top2 = horizontal_damage_.top - 2;
                        const int right2 = horizontal_damage_.right + 2;
                        const int bottom2 = horizontal_damage_.bottom + 2;
                        generate =
                            (row >= top1 && row <= bottom1 &&
                             (left1 > right1 && flat == 0
                                  ? wrapped >= left1 || wrapped < right1
                                  : wrapped >= left1 && wrapped < right1)) ||
                            (row >= top2 && row <= bottom2 &&
                             (left2 > right2 && flat == 0
                                  ? wrapped >= left2 || wrapped < right2
                                  : wrapped >= left2 && wrapped < right2));
                    }
                    if (generate) {
                        int px;
                        int py;
                        tile_to_pixel(wrapped, row, &px, &py);
                        gen_overlays(wrapped, row);
                    }
                }
            }
        }
    }

    if ((field_1DD70_ & 0x20000) != 0) {
        draw_legend();
    }
    if ((field_1DD70_ & 0x100000) != 0) {
        draw_wars();
    }
}

// ORIGINAL: 0x0046A4A0 ?draw_tile@MapWin@@QAEXHHH@Z 0x0046A4A0-0x0046A52F FILE
// working copy - scaffold materialised by --work
// size      143 bytes
// prototype void (__thiscall ?draw_tile@MapWin@@QAEXHHH@Z)(MapWin* this, int xCoord, int yCoord, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046A2A0

void MapWin::draw_tile(int a1, int a2, int a3) {
    char *self = reinterpret_cast<char *>(this);
    int rowStart = field_1DDA8_;
    if (a2 >= rowStart &&
        a2 < field_1DDD8_ + field_1DDD0_ + rowStart) {
        int col = a1;
        if ((MapIsFlat & 1) == 0) {
            int colStart = field_1DDA4_;
            if (a1 < colStart) {
                col = MapLongitudeBounds + a1;
            }
            if (field_1DDD4_ + field_1DDCC_ + colStart <= col) {
                col = col - MapLongitudeBounds;
            }
        }
        int colStart2 = field_1DDA4_;
        if (col >= colStart2 &&
            col < field_1DDD4_ + field_1DDCC_ + colStart2) {
            draw_radius(a1, a2, 0, a3);
        }
    }
}

// ORIGINAL: 0x00470ED0 ?init_dummy@MapWin@@QAEXXZ 0x00470ED0-0x00470F68 FILE
// working copy - scaffold materialised by --work
// size      152 bytes
// prototype void (__thiscall ?init_dummy@MapWin@@QAEXXZ)(MapWin* this)
// callers   1   call targets   2
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2

void MapWin::init_dummy() {
    unsigned char *base;
    int count;
    int offset;
    int i;
    int j;

    field_1DD6C_ = 0x19;
    if (owned_ != 0) {
        free(owned_);
        owned_ = 0;
    }
    owned_ = static_cast<TerrainCacheEntry *>(mem_get((MapArea + MapArea * 2) << 2));
    count = MapArea;
    if (count > 0) {
        offset = 0;
        i = 0;
        do {
            for (j = 1; j <= 4; j++) {
                base = (unsigned char *)owned_;
                base[offset + j - 1] = 0xFF;
            }
            for (j = 1; j <= 5; j++) {
                base = (unsigned char *)owned_;
                base[offset + j + 3] = 0xFF;
            }
            base = (unsigned char *)owned_;
            base[offset + 9] = 0;
            base = (unsigned char *)owned_;
            base[offset + 9] |= (*(unsigned char *)*map_tiles) & 0xD0;
            offset += 0xC;
            i++;
        } while (i < MapArea);
    }
}

static int get_alt_ratio(MapWin * w, int count) {
    if (MapWinCaptionScale < 1) {
        int v = ((int)w->field_1DD98_ + 0x10) * count;
        return (v + ((v >> 0x1f) & 0xf)) >> 4;
    }
    return (MapWinCaptionScale * count) / MapWinCaptionDivisor;
}

// ORIGINAL: 0x0046FE70 ?get_alt@MapWin@@QAEHHHH@Z 0x0046FE70-0x00470414 FILE
// TRIED: get_alt_ratio() helper factoring the repeated scale-ratio computation (4 call sites collapsed to 1 call + reuse); recursive self-call kept inline per original; diverges at insn #3
// working copy - scaffold materialised by --work
// size      1444 bytes
// prototype int (__thiscall ?get_alt@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   8   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046FE70


int MapWin::get_alt(int a1, int a2, int a3) {
    int dirIndex = a3;

    if (a2 < 0 || MapLatitudeBounds <= a2 || a1 < 0 || MapLongitudeBounds <= a1) {
        return 0;
    }
    unsigned char * tile = (unsigned char *)(*map_tiles + (*MapRowStride * a2 + (a1 >> 1)) * 0x2c);
    unsigned char terrain = *tile;
    if ((terrain & 0xe0) < 0x60) {
        return 0;
    }
    if (this->owned_ == 0) {
        return 0;
    }
    char * rec = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * a2 + (a1 >> 1)) * 0xc;
    signed char cached = *(signed char *)(rec + 4 + a3);
    if (cached != -1) {
        return (int)cached;
    }

    unsigned int flags = *(unsigned int *)(tile + 0xc);
    int bonus = 0;
    int dirVal = (int)flags >> 0x18;
    unsigned char byteVal = AltDirectionTable[dirIndex + dirVal * 5];
    if ((flags & 2) != 0) {
        bonus = (int)AltBonusArid[byteVal];
    } else if ((flags & 1) != 0) {
        bonus = (int)AltBonusMoist[byteVal];
    } else if ((flags & 0x200) != 0) {
        bonus = (int)AltBonusRainy[byteVal];
    } else if ((flags & 0x100) != 0) {
        bonus = (int)AltBonusFlat[byteVal];
    } else if ((flags & 0x800) != 0) {
        bonus = (int)AltBonusOcean[byteVal];
    }

    if (dirIndex == 0) {
        int count = this->field_1DD6C_;
        int ratio = get_alt_ratio(this, count);
        int val = (((int)(terrain >> 5)) - 3) * ratio;
        if (bonus < 1) {
            if (bonus < 0) {
                int r = get_alt_ratio(this, count);
                val = val + (r * bonus) / 100;
                if (val < -0x7e) {
                    val = -0x7f;
                }
            }
        } else {
            int r = get_alt_ratio(this, count);
            val = val + (r * bonus) / 100;
            if (val > 0x7e) {
                val = 0x7f;
            }
        }
        int clamped = val;
        if (clamped > 0x7e) {
            clamped = 0x7f;
        }
        rec[4] = (char)clamped;
        return val;
    }

    int iVar9 = dirIndex * 0x10 - 0x10;
    bool useAltMode = false;
    int local_c = 0;
    int local_10 = 0;
    int local_14 = 0;
    int iVar6 = iVar9;
    for (;;) {
        int nx = *(int32_t *)((char *)AltStepX + iVar6) + a1;
        if ((*(unsigned char *)MapIsFlat & 1) == 0) {
            if (nx < 0) {
                nx = MapLongitudeBounds + nx;
            } else if (MapLongitudeBounds <= nx) {
                nx = nx - MapLongitudeBounds;
            }
        }
        int ny = *(int32_t *)((char *)AltStepY + iVar6) + a2;
        if (ny < 0 || MapLatitudeBounds <= ny || nx < 0 || MapLongitudeBounds <= nx ||
            (*(unsigned char *)(*map_tiles + (*MapRowStride * ny + (nx >> 1)) * 0x2c) & 0xe0) < 0x60) {
            break;
        }
        int sub = this->get_alt(nx, ny, 0);
        local_c = local_c + sub;
        local_10 = local_10 + (*(unsigned char *)(*map_tiles + (*MapRowStride * ny + (nx >> 1)) * 0x2c) >> 5);
        local_14 = local_14 + 1;
        iVar6 = iVar6 + 4;
        if (local_14 > 3) {
            goto merge;
        }
    }
    local_c = 0;
    local_10 = 0xc;
    useAltMode = true;

merge:
    local_c = local_c >> 2;
    {
        int count = this->field_1DD6C_;
        if (bonus < 1) {
            if (bonus < 0) {
                int r = get_alt_ratio(this, count);
                local_c = local_c + (r * bonus) / 100;
                if (local_c < -0x7e) {
                    local_c = -0x7f;
                }
            }
        } else {
            int r = get_alt_ratio(this, count);
            local_c = local_c + (r * bonus) / 100;
            if (local_c > 0x7e) {
                local_c = 0x7f;
            }
        }
    }
    dirIndex = -2;
    {
        int width = MapLongitudeBounds;
        int loopCheck;
        do {
            int nx = *(int32_t *)((char *)AltStepX + iVar9) + a1;
            if ((*(unsigned char *)MapIsFlat & 1) == 0) {
                if (nx < 0) {
                    nx = nx + width;
                } else if (width <= nx) {
                    nx = nx - width;
                }
            }
            int ny = *(int32_t *)((char *)AltStepY + iVar9) + a2;
            if (ny >= 0 && ny < MapLatitudeBounds && nx >= 0 && nx < width) {
                nx = nx >> 1;
                if (this->owned_ != 0) {
                    int stored = local_c;
                    if (local_c > 0x7e) {
                        stored = 0x7f;
                    }
                    *((char *)this->owned_ + (dirIndex & 3) + 5 + ((width >> 1) * ny + nx) * 0xc) = (char)stored;
                }
                width = MapLongitudeBounds;
                if (useAltMode) {
                    int slot = (dirIndex & 3) + 1;
                    if (this->owned_ != 0 && slot != 0) {
                        int shift = slot * 4 - 4;
                        char * cell = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * ny + nx) * 0xc;
                        unsigned short * dst = (unsigned short *)(cell + 10);
                        *dst = (unsigned short)((~(0xf << (shift & 0x1f)) & *dst) | (4 << (shift & 0x1f)));
                        width = MapLongitudeBounds;
                    }
                } else {
                    int slot = (dirIndex & 3) + 1;
                    if (this->owned_ != 0 && slot != 0) {
                        int shift = slot * 4 - 4;
                        char * cell = (char *)this->owned_ + ((MapLongitudeBounds >> 1) * ny + nx) * 0xc;
                        unsigned short * dst = (unsigned short *)(cell + 10);
                        unsigned char t = *(unsigned char *)(*map_tiles + (*MapRowStride * ny + nx) * 0x2c);
                        int v = ((local_10 - ((t >> 3) & 0xfffffffc)) + 4) & 0xf;
                        *dst = (unsigned short)((~(0xf << (shift & 0x1f)) & *dst) | (v << (shift & 0x1f)));
                        width = MapLongitudeBounds;
                    }
                }
            }
            iVar9 = iVar9 + 4;
            loopCheck = dirIndex + 3;
            dirIndex = dirIndex + 1;
        } while (loopCheck < 4);
    }
    return local_c;
}

// ORIGINAL: 0x00462F00 ?tile_to_pixel@MapWin@@QAEXHHPAHPAH@Z 0x00462F00-0x0046303C FILE
// symbol    ?tile_to_pixel@MapWin@@QAEXHHPAH0@Z
// size      316 bytes
// prototype void (__thiscall ?tile_to_pixel@MapWin@@QAEXHHPAHPAH@Z)(MapWin* this, int xCoord, int yCoord, int*, int*)
// callers   18   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70
// Original0x00462F4D and0x00462F6B load the receiver's horizontal margin;
// 0x00462F95 reloads the original x argument for the out-of-view fallback.
// The four altitude samples use the unwrapped input coordinates, in4..1 order.
void MapWin::tile_to_pixel(int tile_x, int tile_y, int *pixel_x, int *pixel_y) {
    int map_width = MapLongitudeBounds;
    int relative_x;
    bool in_view = false;
    if (tile_y >= field_1DDA8_ &&
        tile_y < field_1DDD8_ + field_1DDD0_ + field_1DDA8_) {
        int wrapped_x = tile_x;
        int flat = MapIsFlat & 1;
        if (flat == 0) {
            if (wrapped_x < field_1DDA4_) {
                wrapped_x += map_width;
            }
            if (wrapped_x >= field_1DDD4_ + field_1DDCC_ + field_1DDA4_) {
                wrapped_x -= map_width;
            }
        }
        if (wrapped_x >= field_1DDA4_ &&
            wrapped_x < field_1DDD4_ + field_1DDCC_ + field_1DDA4_) {
            in_view = true;
            relative_x = tile_x - field_1DDA4_;
            if (flat == 0) {
                if (relative_x < 0) {
                    relative_x += map_width;
                } else if (relative_x >= map_width) {
                    relative_x -= map_width;
                }
            }
        }
    }
    if (!in_view) {
        relative_x = tile_x - field_1DDA4_;
        if (relative_x > map_width / 2) {
            relative_x -= map_width;
        }
        if (relative_x < -(map_width / 2)) {
            relative_x += map_width;
        }
    }
    *pixel_x = field_1DDBC_ * (relative_x - 1) + field_1DDAC_;
    int altitude = get_alt(tile_x, tile_y, 4);
    altitude += get_alt(tile_x, tile_y, 3);
    altitude += get_alt(tile_x, tile_y, 2);
    altitude += get_alt(tile_x, tile_y, 1);
    *pixel_y = (tile_y - field_1DDA8_ - 1) * field_1DDC0_ +
               field_1DDB0_ - (altitude >> 2);
}

// ORIGINAL: 0x004711A0 ?terrain_fixup@@YAXPAUMapWin@@@Z 0x004711A0-0x0047123B FILE
// symbol    ?terrain_fixup@@YAXPAVMapWin@@@Z
// MapWin is a class here and a struct in the image, so the tag differs.
// TRIED: hoisting `base = owned_+offset` into its own statement before the `j = j+1` increment, to force add-before-inc ordering - same MISMATCH #30 'add' vs 'inc' either way (/O2 schedules inc first regardless of source order). similarity 0.9667, rebuilt_bytes == original_bytes (155).
// working copy - scaffold materialised by --work
// size      155 bytes
// prototype
// callers   1   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005D4510 0x00644EF2
void __cdecl terrain_fixup(MapWin * window) {
    int count2 = 0;
    if (window != 0) {
        if (window->owned_ != 0) {
            free(window->owned_);
            window->owned_ = 0;
        }
        window->owned_ = static_cast<TerrainCacheEntry *>(mem_get(MapArea * 0xc));
        if (MapArea > 0) {
            int offset = 0;
            do {
                int j = 0;
                do {
                    j = j + 1;
                    *(reinterpret_cast<unsigned char *>(window->owned_) + offset - 1 + j) = 0xff;
                } while (j < 4);
                j = 0;
                do {
                    j = j + 1;
                    *(reinterpret_cast<unsigned char *>(window->owned_) + offset + 3 + j) = 0xff;
                } while (j < 5);
                *(reinterpret_cast<unsigned char *>(window->owned_) + 9 + offset) = 0;
                unsigned char *record = reinterpret_cast<unsigned char *>(window->owned_) + 9 + offset;
                offset = offset + 0xc;
                *record = *record | (*map_tile_bytes() & 0xd0);
                count2 = count2 + 1;
            } while (count2 < MapArea);
        }
    }
}

// ORIGINAL: 0x0046FC80 ?UNK6@MapWin@@QAEXHHH@Z 0x0046FC80-0x0046FCD3 FILE
// TRIED: naming the three tables `static int *const` - /O2 then folds the
//   0x66F440-0x66EFBC pair into one walking pointer with an `add`, which the
//   image never does: it keeps each table as its own SIB-indexed base. Spelled
//   `extern int NAME[]` (map.h's arrays) the addresses stay unknown at compile
//   time and the folding cannot happen. MISMATCH #23.
// size      83 bytes
// callers   0   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
// TRIED: `reinterpret_cast<int *>(owned_)[idx * 3]` (one load, the image's
//   shape) versus `(*reinterpret_cast<int **>(owned_))[idx * 3]` (two) - the
//   second scores 26 of 30, the first 21 of 30, because with it VC6 folds
//   `RadiusRange[a3 + 1]` into `[eax*4 + 4]` where the image holds
//   `[eax*4 + 0x66f8c8]`: map.h's RadiusRange is a const header array, so its
//   address is a per-TU constant and the +1 becomes a displacement instead of
//   part of an absolute base. An `extern` RadiusRange would give the image's
//   disp32 shape but moves the array's linkage in every TU that uses it.
void MapWin::UNK6(int UNUSED(x) x, int UNUSED(y) y, int radius) {
    if (owned_ != 0) {
        int i = 0;
        int count = RadiusRange[radius + 1];
        if (count > 0) {
            do {
                // The original ignores x/y: these are offsets around
                // owned_, not around a selected tile. They can be negative,
                // so an ordinary allocation starting at owned_ is unsafe
                // for such radii. No centering, wrapping or guard is added.
                int idx = RadiusOffsetY[i] * (static_cast<int>(MapLongitude) >> 1)
                        + (RadiusOffsetX[i] >> 1);
                i++;
                for (int corner = 0; corner < 4; ++corner) {
                    owned_[idx].brightness[corner] = 0xFF;
                }
            } while (i < count);
        }
    }
}

// ORIGINAL: 0x0046FB90 ?clear_alt@MapWin@@QAEXHHH@Z 0x0046FB90-0x0046FC29 FILE
// TRIED: 153/153 bytes (edit distance 1 pair, same total size).
// Original writes the -1 fill via `or edx,0xffffffff` into a register
// it already holds, then stores that register (2 ops); every source
// form tried here - a direct `*rec = -1`, a named `fill` temp shared
// between the int32 and byte stores, writing the byte store before the
// int32 store - still lowers to a single fused `mov dword ptr[rec],-1`
// (equal cost, different bytes). ~10 tries; not found.
// size      153 bytes
// callers   1   call targets   0
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     (none)
void MapWin::clear_alt(int x, int y, int radius) {
    if (owned_ != 0) {
        int i = 0;
        int count = RadiusRange[radius + 1];
        if (count > 0) {
            int width = MapLongitude;
            do {
                int dx = RadiusOffsetX[i] + x;
                if ((MapIsFlat & 1) == 0) {
                    if (dx < 0) {
                        dx += width;
                    } else if (dx >= width) {
                        dx -= width;
                    }
                }
                int dy = RadiusOffsetY[i] + y;
                if (dy >= 0 && dy < MapLatitudeBounds && dx >= 0 && dx < width) {
                    char *rec = reinterpret_cast<char *>(owned_)
                                + ((width >> 1) * dy + (dx >> 1)) * 0xc + 4;
                    *reinterpret_cast<int32_t *>(rec) = -1;
                    *reinterpret_cast<uint8_t *>(rec + 4) = 0xff;
                    width = MapLongitude;
                }
                i++;
            } while (i < count);
        }
    }
}

// The original invalidates brightness around the cache origin, then resets
// altitude data around the requested tile. UNK6 owns the first typed walk.
// ORIGINAL: 0x0046FD20 ?UNK8@MapWin@@QAEXHHH@Z 0x0046FD20-0x0046FD83 FILE
void MapWin::UNK8(int x, int y, int radius) {
    UNK6(x, y, radius);
    clear_alt(x, y, radius);
}

// ORIGINAL: 0x0046FCE0 ?UNK7@MapWin@@QAEXXZ 0x0046FCE0-0x0046FD1A FILE BYTE_EXACT
// size      58 bytes
// prototype void (__thiscall ?UNK7@MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// PRESERVED UNIT - measured NO_COMPILE.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046FCE0
// measured tier  NO_COMPILE
// refusal        u0046fce0.cpp(1) : error C2653: 'MapWin' : is not a class or namespace name u0046fce0.cpp(2) : error C2065: 'owned_' : undeclared identifier u0046fce0.cpp(3) :
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046fce0/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?UNK7@MapWin@@QAEXXZ  at 0x0046FCE0  (58 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void MapWin::UNK7() {
    if (owned_ != 0) {
        for (int i = 0; i < static_cast<int>(MapArea); ++i) {
            for (int j = 0; j < 4; ++j) {
                reinterpret_cast<char *>(owned_)[i * 0xC + j] = static_cast<char>(0xFF);
            }
        }
    }
}

// ORIGINAL: 0x0046FE00 ?get_relative_alt@MapWin@@QAEHHHH@Z 0x0046FE00-0x0046FE67 FILE BYTE_EXACT
// working copy - scaffold materialised by --work
// size      103 bytes
// prototype int (__thiscall ?get_relative_alt@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70

int MapWin::get_relative_alt(int x, int y, int detail) {
    get_alt(x, y, detail);
    if (owned_ == 0) {
        return 0;
    }
    if (detail == 0) {
        return 4;
    }
    int row = (static_cast<int>(MapLongitude) >> 1) * y + (x >> 1);
    unsigned short val = *reinterpret_cast<unsigned short *>((char *)owned_ + 10 + row * 12);
    int shift = (detail - 1) * 4;
    return (val >> shift) & 0xf;
}

// ORIGINAL: 0x0046AE00 ?draw_cursor@@YAXXZ 0x0046AE00-0x0046AEED FILE
// NOTE ON THE HANDOFF: this was queued as "declare it on MapWin", but the
// image's mangled name is ?draw_cursor@@YAXXZ - a FREE function taking
// nothing, not a member. It walks every slot of the map-window table rather
// than acting on one window, which is what the name is hiding. So it lives at
// file scope beside the other free drawing helpers, not on the class.
//
// TRIED: MapWinTable spelled as the header's `MapWinSlots &` accessor. The
// image walks a live cursor (`add ebx,4 / cmp ebx,0x7d3c5c`) and an index loop
// compares a different thing; the walk is kept, the end bound spelled as the
// table's own one-past-the-end slot.
// size      237 bytes
// callers   2   call targets   2
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046A2A0 0x004BDEA0
// 0x0046AE04 reads ConsoleGlobal's survey mode at 0x9392B0: any active
// survey mode suppresses this cursor pass.

void __cdecl draw_cursor() {
    if (ConsoleGlobal.survey_mode_ == 0) {
        MapWinSlots &table = MapWinTable;
        MapWin **slot = table;
        do {
            MapWin *window = *slot;
            if (window != 0 &&
                (slot == table ||
                 window->field_1DD74_ != 0)) {
                int index = ConsoleGlobal.cursor_history_index_;
                int tile_y = ConsoleGlobal.cursor_history_y_[index];
                int tile_x = ConsoleGlobal.cursor_history_x_[index];
                if (window->field_1DDA8_ <= tile_y &&
                    tile_y < window->field_1DDD8_ +
                             window->field_1DDD0_ +
                             window->field_1DDA8_) {
                    int wrapped_x = tile_x;
                    if ((MapIsFlat & 1) == 0) {
                        if (tile_x < window->field_1DDA4_) {
                            wrapped_x = MapLongitude + tile_x;
                        }
                        if (window->field_1DDD4_ +
                            window->field_1DDCC_ +
                            window->field_1DDA4_ <= wrapped_x) {
                            wrapped_x = wrapped_x - MapLongitude;
                        }
                    }
                    if (window->field_1DDA4_ <= wrapped_x &&
                        wrapped_x < window->field_1DDD4_ +
                                    window->field_1DDCC_ +
                                    window->field_1DDA4_) {
                        window->draw_radius(tile_x, tile_y, 0, 2);
                    }
                }
            }
            slot++;
        } while (slot < table + MapWinTableSlots);
    }
    TutWinGlobal->draw_arrow();
}

// Fixed-slot bindings carried from 00462870.cpp

// ORIGINAL: 0x00462870 ?clear@MapWin@@QAEXH@Z 0x00462870-0x00462959 FILE
// HOMED 2026-09-04 from the 0x00462870 artifact; the artifact's
// `g_9392c0`/`g_939340`/`g_009392b8`/`g_00949870`/`g_00949874` are the
// ConsoleGlobal's cursor_history_x_/cursor_history_y_/cursor_history_index_
// members (console.h) and the tree's
// MapLongitudeBounds / MapLatitudeBounds (map.h), and `g_00939e5c` is the new
// IfaceScreenWidth binding above. Measured in the tree: 8/65.
// TRIED: `g_939340`/`g_9392c0` as the scaffold's `int *const` globals - the address does work (indexed by `*g_009392b8`), so switched to `extern int name[]`. Landed at 97.7%: the original loads `*g_00939e5c` into a register before comparing it to 1000, where this source's compiler folds the load into `cmp dword ptr[addr],1000` directly - tried both an inline dereference and a named local holding the read first; the compiler made the same choice either way. The `((t<1000)-1)&6` then `-6` also comes out as `sub eax,6` here against the original's `add eax,-6` (same value, different opcode).
// TRIED: `int *volatile const IfaceScreenWidth` - volatile on the pointee to
// force the image's `mov edx,[0x939e5c]` ahead of the `cmp` - measured no
// change at all (still 8/65, still the folded memory-operand `cmp`): VC6
// folds a volatile read into a compare operand just as it folds a plain one,
// so volatile buys nothing at this site. Reverted.
// working copy - scaffold materialised by --work
// size      233 bytes
// prototype void (__thiscall ?clear@MapWin@@QAEXH@Z)(MapWin* this, int)
// callers   12   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x00644EF2

void MapWin::clear(int index) {
    field_1DD78_ = index;
    field_1DD70_ = 0;
    field_1DD98_ = (((IfaceScreenWidth < 1000) - 1) & 6) - 6;
    field_1DD74_ = (index == 0);

    int halfHeight = MapLatitudeBounds / 2;
    field_1DDA0_ = halfHeight;
    int halfWidth = MapLongitudeBounds / 2;
    field_1DD9C_ = (halfWidth ^ halfHeight) & 1 ^ halfWidth;

    if (index == 0) {
        int player = ConsoleGlobal.cursor_history_index_;
        int cy = ConsoleGlobal.cursor_history_y_[player];
        int cx = ConsoleGlobal.cursor_history_x_[player];
        if (cy >= 0 && cy < MapLatitudeBounds && cx >= 0 && cx < MapLongitudeBounds) {
            field_1DD9C_ = cx;
            field_1DDA0_ = ConsoleGlobal.cursor_history_y_[ConsoleGlobal.cursor_history_index_];
        }
    }

    cursor_resource_id_ = 0x7f00;
    cursor_sprite_ = 0;
    if (owned_ != 0) {
        free(owned_);
        owned_ = 0;
    }
}


// THE VISIBILITY RECORD, one 0xC-byte slot per half-map column and row, that
// `owned_` points at. Bytes 0..3 hold the four cached corner brightnesses
// (FF = "not computed", 80 = neutral zero) and bytes 10 and 11
// hold the four altitude-detail nibbles `get_alt` leaves behind for the same
// four corners. get_brighting walks it, so the helper is spelled once here and
// expanded at every site - the image computes the address inline at each of
// them, and a plain .cpp function would cost a `call` the image does not make.
static __forceinline char *brighting_tile(void *owned, int row, int col_half) {
    return (char *)owned + ((MapLongitudeBounds >> 1) * row + col_half) * 0xC;
}

// Fixed-slot bindings carried from 00470550.cpp

// ORIGINAL: 0x00470550 ?get_brighting@MapWin@@QAEHHHH@Z 0x00470550-0x00470776 FILE
// HOMED 2026-09-04 from the 0x00470550 artifact. 16/202 in the tree. The
// four absolute table constants 0x7F6680/0x7F66B1/0x7F66E2/0x7F6713 are
// spelled as ONE binding, CornerLightTable, plus the stride 0x31 - the image's
// four displacements are that base plus corner*0x31, and the scorer discounts
// relocations so the single base reproduces the `[reg+reg+disp]` shape.
// TRIED: chained-OR bounds check vs nested ifs, and caching `this` in a named local before the owned_==0 check - none change when the compiler spills `this` to [ebp-4] relative to the row/col bound-check jumps (MISMATCH #6 'push' vs 'mov'); not chased further. Switch-table index math (v1..v4 nibble packing, per-case table*7 stride) is a faithful transcription.
// working copy - scaffold materialised by --work
// size      550 bytes
// prototype int (__thiscall ?get_brighting@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x0046FE70


int MapWin::get_brighting(int x, int y, int corner) {
    int row = y;
    int col = x;
    if (row < 0 || row >= MapLatitudeBounds || col < 0 || col >= MapLongitudeBounds) {
        return 0;
    }
    if (owned_ == 0) {
        return -0x80;
    }
    int colHalf = col >> 1;
    char *tile = brighting_tile(owned_, row, colHalf);
    unsigned int v0 = (unsigned char)tile[corner];
    char *tileByteAddr = tile + corner;
    if (v0 != 0xff) {
        return (int)v0 - 0x80;
    }
    unsigned char terrain = *(unsigned char *)map_loc(col, row);
    if ((terrain & 0xe0) < 0x60) {
        *tileByteAddr = (char)0x80;
        return 0;
    }
    get_alt(col, row, 1);
    int v1;
    if (owned_ == 0) {
        v1 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v1 = (unsigned char)t[10] & 0xf;
    }
    get_alt(col, row, 2);
    int v2;
    if (owned_ == 0) {
        v2 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v2 = ((unsigned char)t[10] >> 4) & 0xf;
    }
    get_alt(col, row, 3);
    int v3;
    if (owned_ == 0) {
        v3 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v3 = (unsigned char)t[11] & 0xf;
    }
    get_alt(col, row, 4);
    int v4;
    if (owned_ == 0) {
        v4 = 0;
    } else {
        char *t = brighting_tile(owned_, row, colHalf);
        v4 = (unsigned char)t[11] >> 4;
    }
    int result;
    switch (corner) {
    case 0: {
        const int index = v1 * 7 + v2;
        if (index < 4) {
            // Original470704's biased base7F6680 selects the bytes of the
            // last ocean shade. Decode the x86 little-endian signed byte
            // without an out-of-bounds array or a separate shadow owner.
            const unsigned int byte =
                (static_cast<unsigned int>(OceanCornerShades[15]) >>
                 (index * 8)) & 0xffU;
            result = byte < 0x80U ? static_cast<int>(byte)
                                  : static_cast<int>(byte) - 0x100;
        } else {
            result = corner_light_value(index);
        }
        break;
    }
    case 1:
        result = corner_light_value(0x31 + v2 * 7 + v3);
        break;
    case 2:
        result = corner_light_value(0x62 + v3 * 7 + v4);
        break;
    case 3:
        result = corner_light_value(0x93 + v4 * 7 + v1);
        break;
    default:
        result = (int)v0;
        break;
    }
    char *finalTile = brighting_tile(owned_, row, colHalf);
    finalTile[corner] = (char)(result + 0x80);
    return result;
}

// ORIGINAL: 0x00470420 ?compute_lighting_table@MapWin@@QAEXXZ 0x00470420-0x00470536
// RECOVERY_BINDING: v1 {"evidence": ["The original 0x00470420..0x00470536 body fills four 49-byte tables using quick_root; its retired transcription remains in Git history", "src/mapwin.cpp: MapLightingTable bounds 0x007F6688..0x007F674C and CornerLightTable consumer", "src/general.h: quick_root(int)"], "reason": "Build the four 7-by-7 signed corner-light tables in their source-owned storage using the existing integer-root routine."}
void MapWin::compute_lighting_table() {
    fill_map_lighting_table();
}

// Fixed-slot bindings carried from 00470790.cpp
// ORIGINAL: 0x00470790 ?get_point_light@MapWin@@QAEHHHH@Z 0x00470790-0x00470917 FILE
// HOMED 2026-09-04 from the 0x00470790 artifact. 22/139 under /O2 /GX.
// The scaffold's seven fixed globals all have names in this file now:
// g_00685684 = AltDirectionTable, g_00685A24/g_00685B60/g_007F6750 =
// AltLightBit1/AltLightBit2/AltLightBit200, g_00685EA0/g_00685EE0 = the two
// CornerOffset tables map.h defines, g_0094988C = MapIsFlat (map.h), and the
// tile read is `*map_tiles`/`*MapRowStride`. Two spellings measured:
// TRIED: `map_loc(a1, a2)->bit2` for the tile read - 6/139, and the image's
//            `mov eax,[0x68faf0]` came out relocation-masked to `[0]`; the
//            explicit `*map_tiles` + `*MapRowStride` form reads the same two
//            globals in the image's order and scored 9/139.
// LEVER: the neighbour walk as an up-counting `for (j = 0; j < 4; ++j)` over
//            `CornerOffsetX[a3-1]` / `CornerOffsetY[a3-1]` with the corner
//            arguments `(j+1)&3` and `(j+2)&3`, instead of the do/while that
//            carries a down-counting 0xfffffffe counter and its `check` temp
//            - 9/139 -> 22/139, and the frame drops two slots.
// TRIED: none confirmed byte-exact; MISMATCH #4 'mov' vs 'push' - the do/while neighbour-sum loop's register pressure does not reproduce the original's ebp-frame local layout.
// size      391 bytes
// prototype int (__thiscall ?get_point_light@MapWin@@QAEHHHH@Z)(MapWin* this, int, int, int)
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x00470550

int MapWin::get_point_light(int x, int y, int corner) {
    int total = 0;
    int localLight = 0;

    unsigned int rec =
        *reinterpret_cast<const unsigned int *>(reinterpret_cast<const uint8_t *>(*map_tiles) +
            (*MapRowStride * y + (x >> 1)) * 0x2c + 0xc);
    int shade = static_cast<int>(rec) >> 0x18;

    if ((rec & 1) == 0) {
        if ((rec & 2) == 0) {
            if ((rec & 0x200) != 0 && shade >= 0) {
                unsigned char idx = AltDirectionTable[corner + shade * 5];
                localLight = AltLightBit200[idx];
            }
        } else if (shade >= 0) {
            unsigned char idx = AltDirectionTable[corner + shade * 5];
            localLight = AltLightBit2[idx];
        }
    } else if (shade >= 0) {
        unsigned char idx = AltDirectionTable[corner + shade * 5];
        localLight = AltLightBit1[idx];
    }

    if (corner == 0) {
        for (int i = 0; i < 4; ++i) {
            total += get_brighting(x, y, i);
        }
        return (total >> 2) + localLight;
    }

    const int (*corner_x)[4] = CornerOffsetX + corner - 1;
    const int (*corner_y)[4] = CornerOffsetY + corner - 1;
    for (int j = 0; j < 4; ++j) {
        int px = x + (*corner_x)[j];
        if ((MapIsFlat & 1) == 0) {
            if (px < 0) {
                px += MapLongitudeBounds;
            } else if (px >= MapLongitudeBounds) {
                px -= MapLongitudeBounds;
            }
        }
        int py = y + (*corner_y)[j];
        total += get_brighting(px, py, (j + 1) & 3);

        px = x + (*corner_x)[j];
        if ((MapIsFlat & 1) == 0) {
            if (px < 0) {
                px += MapLongitudeBounds;
            } else if (px >= MapLongitudeBounds) {
                px -= MapLongitudeBounds;
            }
        }
        total += get_brighting(px, py, (j + 2) & 3);
    }

    return (total >> 3) + localLight;
}

// Fixed-slot bindings carried from 00462980.cpp

// ORIGINAL: 0x00462980 ?calculate_dimensions@MapWin@@QAEXXZ 0x00462980-0x00462EFA FILE
// size      1402 bytes
// prototype void (__thiscall ?calculate_dimensions@MapWin@@QAEXXZ)(MapWin* this)
// callers   3   call targets   1
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     0x00618F40
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x00462980
// measured tier  MISMATCH
// divergence     1
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/00462980/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?calculate_dimensions@MapWin@@QAEXXZ  at 0x00462980  (1402 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void MapWin::calculate_dimensions() {
    field_1DDAC_ = 0;
    field_1DDB0_ = 0;

    int32_t v;
    if (MapWinCaptionScale > 0) {
        v = (MapWinCaptionScale * 50) / MapWinCaptionDivisor;
    } else {
        v = ((field_1DD98_ + 0x10) * 50) / 16;
    }
    v = (v + 1) / 4;
    field_1DDC8_ = v;
    if (v < 1) {
        field_1DDC8_ = 1;
    }
    int32_t T = field_1DDC8_;
    int32_t c2 = T * 2;
    field_1DDC4_ = c2;
    int32_t c4a = c2 * 2;
    int32_t c8 = c2 * 4;
    field_1DDBC_ = c4a;
    field_1DDC0_ = c2;
    field_1DDB4_ = c8;
    field_1DDB8_ = T * 4;

    char *const entered = entered_receiver(this);
    int32_t w = *reinterpret_cast<int32_t *>(entered + 0x4c4);
    int32_t h = *reinterpret_cast<int32_t *>(entered + 0x4c8);

    field_1DDCC_ = (w + c8 - 1) / c8;
    field_1DDD0_ = ((T * 4 - h) + c2 - 1) / (T * 4);
    field_1DDD4_ = (w + T * 4 + c8 - 1) / c8;
    field_1DDD8_ = ((T * 4 - h) + c2 - 1) / (T * 4);

    if (this == reinterpret_cast<MapWin *>(&ConsoleGlobal)) {
        int32_t a = field_1DDA0_;
        int32_t b = field_1DD9C_;
        MapWinPrimaryCorner.left = b;
        MapWinPrimaryCorner.top = a;
        MapWinPrimaryCorner.right = b;
        MapWinPrimaryCorner.bottom = a;
        int32_t f73 = field_1DDCC_;
        int32_t f74 = field_1DDD0_;
        int32_t e1 = (b - f73) + 4;
        int32_t e2 = (f73 + b) - 4;
        int32_t e3 = (a - f74) + 4;
        int32_t e4 = (a + f74) - 6;
        MapWinPrimaryCorner.left = e1;
        MapWinPrimaryCorner.right = e2;
        MapWinPrimaryCorner.top = e3;
        MapWinPrimaryCorner.bottom = e4;
    }

    int32_t halfW = MapLongitudeBounds / 2;
    if (field_1DDCC_ > halfW) {
        field_1DDCC_ = halfW;
        int32_t tileProduct = field_1DDCC_ * field_1DDB4_;
        field_1DDD4_ = halfW + 1;
        field_1DDAC_ = (w - tileProduct) / 2;
    }
    int32_t halfH = MapLatitudeBounds / 2;
    if (field_1DDD0_ > halfH) {
        field_1DDD0_ = halfH;
        int32_t tileProduct2 = field_1DDD0_ * field_1DDB8_;
        field_1DDD8_ = halfH + 1;
        field_1DDB0_ = -((h + tileProduct2) / 2);
    }

    int32_t halfW2 = MapLongitudeBounds / 2;
    field_1DDCC_ = (field_1DDCC_ < halfW2) ? field_1DDCC_ : halfW2;
    int32_t halfW3 = MapLongitudeBounds / 2;
    field_1DDD4_ = (field_1DDD4_ < halfW3) ? field_1DDD4_ : halfW3;

    int32_t f74_ = field_1DDD0_;
    int32_t aVal = field_1DDA0_;
    int32_t panYinit = aVal - f74_;
    field_1DDA8_ = panYinit;
    int32_t altY = MapLatitudeBounds + (2 - f74_) * 2;
    int32_t panY;
    if (panYinit < -2 || altY < -2) {
        panY = -2;
    } else if (altY < panYinit) {
        panY = altY;
    } else {
        panY = panYinit;
    }
    field_1DDA8_ = panY;

    int32_t f73_ = field_1DDCC_;
    int32_t bVal = field_1DD9C_;
    int32_t panXinit = bVal - f73_;
    uint8_t wrapFlag = *reinterpret_cast<uint8_t *>(&MapIsFlat);
    int32_t panX;
    if ((wrapFlag & 1) == 0) {
        int32_t screenW = MapLongitudeBounds;
        if (panXinit < 0) {
            panX = panXinit + screenW;
        } else if (panXinit >= screenW) {
            panX = panXinit - screenW;
        } else {
            panX = panXinit;
        }
    } else {
        panX = panXinit;
    }
    field_1DDA4_ = panX;

    if ((wrapFlag & 1) != 0) {
        int32_t altX = MapLongitudeBounds + (2 - f73_) * 2;
        if (panX < -4 || altX < -4) {
            panX = -4;
        } else if (altX < panX) {
            panX = altX;
        }
        field_1DDA4_ = panX;
    }

    field_1DDA4_ = ((field_1DDA4_ ^ field_1DDA8_) & 1) ^ field_1DDA4_;

    int32_t fogW;
    if ((field_1DD70_ & 2) == 0 || field_1DD98_ < -10) {
        fogW = 0;
    } else if (MapWinCaptionScale < 1) {
        fogW = ((field_1DD98_ * 3 + 0x30) * 4) / 16;
    } else {
        fogW = (MapWinCaptionScale * 12) / MapWinCaptionDivisor;
    }
    field_1DDDC_ = fogW;

    int32_t fogH;
    if ((field_1DD70_ & 2) == 0 || field_1DD98_ < -10) {
        fogH = 0;
    } else if (MapWinCaptionScale < 1) {
        fogH = (field_1DD98_ * 4 + 0x40) / 16;
    } else {
        fogH = (MapWinCaptionScale * 4) / MapWinCaptionDivisor;
    }
    field_1DDE0_ = fogH;

    field_1DD6C_ = 0x19;
    if ((GameDataStateStorage.fields_.game_more_preferences_ & 0x10000) != 0) {
        field_1DD6C_ = 8;
        field_1DDDC_ = fogW / 3;
        field_1DDE0_ = fogH / 3;
    }

    if (field_1DD6C_ != field_1EEC4_) {
        field_1EEC4_ = field_1DD6C_;
        if ((int)(intptr_t)owned_ != 0) {
            int32_t count = (int32_t)MapArea;
            char *base = reinterpret_cast<char *>(owned_);
            for (int32_t ia = 0; ia < count; ++ia) {
                for (int32_t ja = 0; ja < 5; ++ja) {
                    base[ia * 0xc + ja + 4] = static_cast<char>(0xff);
                }
            }
        }
    }

    if (field_1DD98_ != field_1EEC0_) {
        if ((int)(intptr_t)owned_ != 0) {
            int32_t count = (int32_t)MapArea;
            char *base = reinterpret_cast<char *>(owned_);
            for (int32_t ib = 0; ib < count; ++ib) {
                for (int32_t jb = 0; jb < 4; ++jb) {
                    base[ib * 0xc + jb] = static_cast<char>(0xff);
                }
            }
            count = (int32_t)MapArea;
            base = reinterpret_cast<char *>(owned_);
            for (int32_t ic = 0; ic < count; ++ic) {
                for (int32_t jc = 0; jc < 5; ++jc) {
                    base[ic * 0xc + jc + 4] = static_cast<char>(0xff);
                }
            }
        }
        field_1EEC0_ = field_1DD98_;

        int32_t fs1 = (field_1DDB8_ * 5) / 16;
        if (fs1 > 0xc) {
            fs1 = 0xc;
        }
        if (field_1EEF0_ != fs1) {
            field_1EEF0_ = fs1;
            font_a_.init(reinterpret_cast<char *>(*DefaultFontFace), fs1, 0);
        }
        int32_t fs2 = (field_1DDB8_ * 5) / 16;
        if (fs2 > 0xc) {
            fs2 = 0xc;
        }
        if (field_1EF48_ != fs2) {
            field_1EF48_ = fs2;
            font_c_.init(reinterpret_cast<char *>(*DefaultFontFace), fs2, 2);
        }
        int32_t fs3 = (field_1DDB8_ * 2) / 5;
        if (fs3 > 0xc) {
            fs3 = 0xc;
        }
        if (field_1EF1C_ != fs3) {
            field_1EF1C_ = fs3;
            font_b_.init(reinterpret_cast<char *>(*DefaultFontFace), fs3, 1);
        }
    }

#undef I32
}

// ORIGINAL: 0x00463040 ?pixel_to_tile@MapWin@@QAEHHHPAHPAH@Z 0x00463040-0x004632CC FILE
// symbol    ?pixel_to_tile@MapWin@@QAEHHHPAH0@Z
//           VC6 merges the two identical `int *` argument types into the
//           back-reference `0`, so the definition emits `PAH0` where the
//           catalogue spells `PAHPAH`. Same two pointers, one mangling.
// TRIED: MISMATCH #10 'lea' vs 'cmp' - transcribed the Ghidra pseudocode almost verbatim (including the vbtable-indirected bounds check via *(int*)(*this+4)); the entry sequence loading eax/ecx from [edi] and [eax+4] diverges around instruction 10, not chased further.
// working copy - scaffold materialised by --work
// size      652 bytes
// prototype int (__thiscall ?pixel_to_tile@MapWin@@QAEHHHPAHPAH@Z)(MapWin* this, int, int, int*, int*)
// callers   7   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x0046FE70

int MapWin::pixel_to_tile(int x_pixel, int y_pixel, int *out_tile_x,
                          int *out_tile_y) {
    // The GraphicWin virtual base's client width (0x4C4) and height (0x4C8).
    char *const entered = entered_receiver(this);
    if ((x_pixel < *reinterpret_cast<int *>(entered + 0x4C4)) &&
        (y_pixel < -*reinterpret_cast<int *>(entered + 0x4C8))) {
        x_pixel = x_pixel - field_1DDAC_;
        y_pixel = y_pixel - field_1DDB0_;
        if ((-1 < x_pixel) && (-1 < y_pixel)) {
            int iVar1;
            int iVar2;
            int iVar3;
            int iVar4;
            int iVar5;
            unsigned int uVar6;
            int iVar7;
            int iVar8;
            bool bVar9;
            int local_8;

            iVar5 = field_1DDA4_;
            iVar3 = iVar5 + 1 + (x_pixel / field_1DDB4_) * 2;
            uVar6 = MapIsFlat & 1;
            if (uVar6 == 0) {
                if (iVar3 < 0) {
                    iVar3 = MapLongitudeBounds + iVar3;
                } else if (MapLongitudeBounds <= iVar3) {
                    iVar3 = iVar3 - MapLongitudeBounds;
                }
            }
            iVar1 = field_1DDBC_;
            iVar2 = iVar5 + 1 + ((x_pixel - iVar1) / field_1DDB4_) * 2;
            if (uVar6 == 0) {
                if (iVar2 < 0) {
                    iVar2 = iVar2 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar2) {
                    iVar2 = iVar2 - MapLongitudeBounds;
                }
            }
            bVar9 = iVar2 != iVar3;
            if (bVar9) {
                local_8 = iVar3 - 1;
            } else {
                iVar3 = iVar3 + 1;
                local_8 = iVar3;
            }
            bVar9 = !bVar9;
            iVar3 = iVar3 - iVar5;
            iVar5 = iVar3 - 1;
            if (uVar6 == 0) {
                if (iVar5 < 0) {
                    iVar5 = iVar5 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar5) {
                    iVar5 = iVar5 - MapLongitudeBounds;
                }
            }
            if (uVar6 == 0) {
                if (iVar3 < 0) {
                    iVar3 = iVar3 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= iVar3) {
                    iVar3 = iVar3 - MapLongitudeBounds;
                }
            }
            iVar2 = field_1DDA8_ + (y_pixel / field_1DDB8_) * 2;
            do {
                iVar8 = iVar2;
                bVar9 = (bVar9 == false);
                if (bVar9) {
                    iVar2 = this->get_alt(local_8, iVar8, 2);
                    iVar2 = ((iVar8 - field_1DDA8_) - 1) * field_1DDC0_ - iVar2;
                    iVar4 = this->get_alt(local_8, iVar8, 3);
                    iVar4 = (iVar8 - field_1DDA8_) * field_1DDC0_ - iVar4;
                    local_8 = local_8 + 1;
                } else {
                    iVar2 = this->get_alt(local_8, iVar8, 1);
                    iVar2 = (iVar8 - field_1DDA8_) * field_1DDC0_ - iVar2;
                    iVar4 = this->get_alt(local_8, iVar8, 2);
                    iVar4 = ((iVar8 - field_1DDA8_) - 1) * field_1DDC0_ - iVar4;
                    local_8 = local_8 - 1;
                }
                iVar4 = (iVar4 - iVar2) * (x_pixel - iVar1 * iVar5);
                iVar7 = (y_pixel - iVar2) * (iVar1 * iVar3 - iVar1 * iVar5);
                iVar2 = iVar8 + 1;
            } while (iVar4 - iVar7 == 0 || iVar4 < iVar7);
            iVar8 = iVar8 - 1;
            if ((MapIsFlat & 1) == 0) {
                if (local_8 < 0) {
                    local_8 = local_8 + MapLongitudeBounds;
                } else if (MapLongitudeBounds <= local_8) {
                    local_8 = local_8 - MapLongitudeBounds;
                }
            }
            *out_tile_x = local_8;
            *out_tile_y = iVar8;
            if ((-1 < iVar8) && (iVar8 < MapLatitudeBounds) && (-1 < *out_tile_x) && (*out_tile_x < MapLongitudeBounds)) {
                return 0;
            }
        }
    }
    return 1;
}

// Fixed-slot bindings carried from 0046ec10.cpp

typedef short (__stdcall *KeyStateFn)(int);

class Slot17Shim { public:
    virtual void slot00(); virtual void slot01(); virtual void slot02();
    virtual void slot03(); virtual void slot04(); virtual void slot05();
    virtual void slot06(); virtual void slot07(); virtual void slot08();
    virtual void slot09(); virtual void slot10(); virtual void slot11();
    virtual void slot12(); virtual void slot13(); virtual void slot14();
    virtual void slot15(); virtual void slot16();
    virtual void slot17(int, int);  // <-- used, offset 0x44
};

// ORIGINAL: 0x0046EC10 ?on_right_down@MapWin@@QAEXHH@Z 0x0046EC10-0x0046ED21 FILE
// symbol    ?on_right_down@MapWin@@UAEXHH@Z
// size      273 bytes
// prototype void (__thiscall ?on_right_down@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x004B4A20
// indirect  0x0046EC76 0x0046EC82 0x0046EC91 0x0046EC9D 0x0046ED0C 0x0046ED18
// PRESERVED UNIT - measured MNEMONIC_ONLY.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046EC10
// measured tier  MNEMONIC_ONLY
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046ec10/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_right_down@MapWin@@QAEXHH@Z  at 0x0046EC10  (273 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void MapWin::on_right_down(int x_coord, int y_coord) {
    if (MapInputEnabled == 0) {
        return;
    }

    if (this->field_1DD80_ != 0) {
        ConsoleStatusWin.set_title(0);
    }

    this->field_1DD7C_ = 0;
    this->field_1DD80_ = 0;
    this->field_1DD8C_ = 0;
    this->field_1DD84_ = 0;
    this->field_1DE0C_ = -1;
    this->field_1DD88_ = 0;

    if (ConsoleGlobal.field_23D10_ != 0) {
        return;
    }

    KeyStateFn get_key_state = *reinterpret_cast<KeyStateFn *>(IAT_GetAsyncKeyState);

    uint8_t shift_down = 0;
    shift_down = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x10)) >> 8);
    if (shift_down) {
        uint8_t ctrl_down = 0;
        ctrl_down = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x11)) >> 8);
        if (ctrl_down) {
            goto check_flag;
        }
    }

    if (!((*reinterpret_cast<KeyStateFn *>(IAT_GetKeyState))(0x91) & 1)) {
        goto abort;
    }

    {
        uint8_t ctrl_down2 = 0;
        ctrl_down2 = static_cast<uint8_t>(static_cast<uint16_t>(get_key_state(0x11)) >> 8);
        if (ctrl_down2) {
            goto abort;
        }
    }

check_flag:
    if (GameDataStateStorage.fields_.game_state_ & 0x40) {
        this->field_1DD8C_ = 2;
        this->field_1DD94_ = -1;
        this->field_1DD90_ = -1;

        char *const entered = entered_receiver(this);

        if (WinPointerOwner3 == 0) {
            WinPointerOwner4 = 0;
            WinPointerOwner3 = reinterpret_cast<Win *>(
                *reinterpret_cast<int32_t *>(entered) + entered);
        }

        {
            int32_t offset =
                *reinterpret_cast<int32_t *>(entered + MapWinEnteredOffset);
            reinterpret_cast<Slot17Shim *>(entered + offset)->slot17(x_coord, y_coord);
        }
        return;
    }

abort:
    get_key_state(0x11);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0046ED30 ?on_left_down@MapWin@@QAEXHH@Z 0x0046ED30-0x0046EFE1 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0046ed30.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
// symbol    ?on_left_down@MapWin@@UAEXHH@Z
void MapWin::on_left_down(int x_coord, int y_coord) {
    if (MapInputEnabled == 0) return;

    go_reset();
    field_1DD7C_ = 0;
    field_1DD80_ = 0;
    field_1DD8C_ = 0;
    field_1DE0C_ = -1;
    field_1DD88_ = 0;
    field_1DD84_ = 0;

    if (ConsoleControlTurnActive != 0 || field_1DD78_ != 0) return;
    if (ConsoleGlobal.field_23D10_ != 0) {
        if (ConsoleGlobal.field_23D10_ != 8) return;
        field_1DD80_ = 1;
        if (WinPointerOwner3 == nullptr) {
            WinPointerOwner4 = nullptr;
            WinPointerOwner3 = static_cast<Win *>(this);
        }
        CoverWindow.fill(9);
        CoverWindow.show(0);
        on_mouse_move(x_coord, y_coord);
        return;
    }

    const bool shift_down =
        (static_cast<uint16_t>(GetAsyncKeyState(VK_SHIFT)) & 0xFF00) != 0;
    if (!shift_down &&
        (static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0xFF00) != 0) {
        goto check_undo;
    }
    if (!(GetKeyState(VK_SCROLL) & 1)) {
        goto after_undo;
    }
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0xFF00) != 0) {
        goto after_undo;
    }

check_undo:
    if ((GameDataStateStorage.fields_.game_state_ & 0x40) != 0) {
        auto_undo();
        field_1DD8C_ = 1;
        field_1DD94_ = -1;
        field_1DD90_ = -1;
        if (WinPointerOwner3 == nullptr) {
            WinPointerOwner4 = nullptr;
            WinPointerOwner3 = static_cast<Win *>(this);
        }
        on_mouse_move(x_coord, y_coord);
        return;
    }

after_undo:
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0xFF00) != 0)
        return;
    if (ConsoleGlobal.edit_lock() != 0) return;

    int tile_x = 0;
    int tile_y = 0;
    if (ConsoleGlobal.cursor_active_ == 0 || (field_1DD70_ & 0x40000U) != 0) {
        if (pixel_to_tile(x_coord, y_coord, &tile_x, &tile_y) != 0 ||
            tile_x < 0 || tile_x >= static_cast<int>(MapLongitudeBounds) ||
            tile_y < 0 || tile_y >= static_cast<int>(MapLatitudeBounds)) {
            return;
        }
        if ((field_1DD70_ & 0x40000U) != 0) {
            field_1DE24_ = base_find(tile_x, tile_y, ConsoleGlobal.faction_id_);
            if (field_1DE24_ < 0) return;
        }
    }

    MapWinSelectedSlot = static_cast<int>(field_1DD78_);
    if ((static_cast<uint16_t>(GetAsyncKeyState(VK_SHIFT)) & 0xFF00) != 0 &&
        (GameDataStateStorage.fields_.game_state_ & 0x40) == 0) {
        go_timer(3);
        return;
    }

    const int guard = ConsoleGlobal.cursor_active_ != 0 ? -1 : 0;
    const int timer_parameter = (guard & 0xFFFFFD65) + 0x3E8;
    GoTimer.start(go_timer, 3, timer_parameter, 5);
}

// Superseded seed deleted; seed paths below are historical Git provenance.
// ORIGINAL: 0x0050F170 ?go_timer@@YAXH@Z 0x0050F170-0x0050F3C4 FILE
// RECOVERY_BINDING: v1 {"evidence": ["src/recovered/units/0050f170.cpp", "Pinned original image review controls route precedence, cursor sprite dispatch, signed zoom, actual API imports and original member/data dependencies. Bounded compile and host controls do not establish game acceptance."], "reason": "Recover the genuine MapWin/Console owner operation using typed members, actual shared storage, virtual base APIs and full original branch evidence; keep seed and historical obligations."}
void __cdecl go_timer(int selection_mode) {
    GoTimer.close();
    if (MapWinSelectedSlot < 0 || TimerHoldFlag != 0) return;

    MapWin *const window = MapWinTable[MapWinSelectedSlot];
    int mouse_x;
    int mouse_y;
    static_cast<Win *>(window)->get_mouse_pos(&mouse_x, &mouse_y);

    const bool control_down =
        (static_cast<uint16_t>(GetAsyncKeyState(VK_CONTROL)) & 0xFF00) != 0;
    // The key branch defaults to cursor 1. With Control held and no active
    // cursor, a clear 0x23D1C selects cursor 9; map-wait mode also forces 9.
    int mode = control_down && ConsoleGlobal.cursor_active_ == 0 &&
               ConsoleGlobal.field_23D1C_ == 0 ? 9 : 1;

    int new_vehicle = ConsoleGlobal.selected_vehicle_;
    const int history = ConsoleGlobal.cursor_history_index_;
    if ((window->field_1DD70_ & 0x40000U) == 0) {
        const bool must_reselect =
            mode == 1 && ConsoleGlobal.cursor_active_ == 0 &&
            ConsoleGlobal.field_23D1C_ == 0 &&
            (new_vehicle < 0 ||
             Vehs[new_vehicle].x != ConsoleGlobal.cursor_history_x_[history] ||
             Vehs[new_vehicle].y != ConsoleGlobal.cursor_history_y_[history] ||
             veh_selectable(new_vehicle) == 0 || Vehs[new_vehicle].order != 0 ||
             (Vehs[new_vehicle].state & 0x4200U) != 0);
        if (must_reselect) {
            new_vehicle = veh_find(ConsoleGlobal.cursor_history_x_[history],
                                   ConsoleGlobal.cursor_history_y_[history],
                                   ConsoleGlobal.faction_id_, -1);
            if (new_vehicle < 0 || veh_selectable(new_vehicle) == 0 ||
                Vehs[new_vehicle].order != 0 ||
                (Vehs[new_vehicle].state & 0x200U) != 0 ||
                (Vehs[new_vehicle].state & 0x4000U) == 0) {
                go_reset();
                return;
            }
        }
    } else {
        mode = 9;
    }

    ConsoleGlobal.selected_vehicle_ = new_vehicle;
    Sprite *const cursor = &g_CURSOR_SPRITES[mode];
    if (window->cursor_sprite_ != cursor) {
        window->set_cursor(cursor, CursorHotspotX[mode], CursorHotspotY[mode]);
        window->cursor_sprite_ = cursor;
    }

    if (WinPointerOwner3 == nullptr) {
        WinPointerOwner4 = nullptr;
        WinPointerOwner3 = static_cast<Win *>(window);
    }
    CoverWindow.fill(9);
    CoverWindow.show(0);
    window->field_1DD80_ = 1;
    window->field_1DD84_ =
        (ConsoleGlobal.cursor_active_ == 0 && ConsoleGlobal.field_23D1C_ != 0)
            ? 1U : 0U;
    window->on_mouse_move(mouse_x, mouse_y);
}

class VCallI { public:
    virtual void slot000();
    virtual void slot001();
    virtual void slot002();
    virtual void slot003();
    virtual void slot004();
    virtual void slot005();
    virtual void slot006();
    virtual void slot007();
    virtual void slot008();
    virtual void slot009();
    virtual void slot010();
    virtual void slot011();
    virtual void slot012();
    virtual void slot013();
    virtual void slot014();
    virtual void slot015();
    virtual void slot016();
    virtual void slot017();
    virtual void slot018();
    virtual void slot019();
    virtual void slot020();
    virtual void slot021();
    virtual void slot022();
    virtual void slot023();
    virtual void slot024();
    virtual void slot025();
    virtual void slot026();
    virtual void slot027();
    virtual void slot028();
    virtual void slot029();
    virtual void slot030();
    virtual void slot031();
    virtual void slot032();
    virtual void slot033();
    virtual void slot034();
    virtual void slot035();
    virtual void slot036();
    virtual void slot037();
    virtual void slot038();
    virtual void slot039();
    virtual void slot040();
    virtual void slot041();
    virtual void slot042();
    virtual void slot043();
    virtual void slot044(int);  // <-- used, takes the region id
};

// ORIGINAL: 0x0046F700 ?on_nc_left_down@MapWin@@QAEXHH@Z 0x0046F700-0x0046F880 FILE
// size      384 bytes
// prototype void (__thiscall ?on_nc_left_down@MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FA7E0
// indirect  0x0046F784 0x0046F7D3 0x0046F822 0x0046F871
// PRESERVED UNIT - measured MISMATCH.
//
// Kept for COVERAGE. This directory IS on the ratchet: every file here
// carries an ORIGINAL marker, `decomp_status.py` compiles and measures
// it, and 336 of the 1,108 now carry a BYTE_EXACT claim - better than a
// quarter of the project's total. It is still in no build; the earlier
// header said "on no ratchet", which stopped being true when the map
// moved into src/ and was still being written into new files.
//
// address        0x0046F700
// measured tier  MISMATCH
// divergence     11
//
// The WHOLE unit as measured, scaffolding included: for the units
// that are byte-exact yet refuse extraction, the agent tuned the
// emitted scaffolding and the body alone will not reproduce the
// verdict. To resume, copy everything below back over
//   build/byte-match/0046f700/unit.cpp
// and score it with tools/agent_brief.py.
// GENERATED SKELETON - tools/emit_translation_unit.py
// subject: ?on_nc_left_down@MapWin@@QAEXHH@Z  at 0x0046F700  (384 bytes)
//
// A VERIFICATION ARTIFACT, not product source: classes are opaque and
// globals are bound to fixed addresses, because both are byte-visible
// and both differ from the style src/ is written in.
//
// The VC6 dialect limits and the source-form rules used to live here.
// They are knowledge, not scaffolding, so they now live in the agent
// system prompt (mizuchi.yaml, plugins.claude-runner.systemPrompt),
// where they can be edited without regenerating anything and are in
// context from the first token rather than behind a file read. This
// emitter computes declarations; it does not carry lessons.


void MapWin::on_nc_left_down(int x_coord, int y_coord) {
    RECT rect = {0, 0xe, 0x11, 0x1f};

    // `entered` is the GraphicWin virtual base the image hands this
    // handler: front + 0x21A6C. Every offset below is relative to it.
    char *const entered = entered_receiver(this);

    int32_t t1 = *reinterpret_cast<int32_t *>(
        *reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4);
    int32_t fA = *reinterpret_cast<int32_t *>(entered + t1 - 0x21918);
    int32_t fB = *reinterpret_cast<int32_t *>(entered + t1 - 0x21920);
    int32_t diff = fA - fB;
    diff -= 0x5b;
    rect.left = diff;
    rect.right = diff + 0x11;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(3);
        return;
    }

    rect.left = rect.left + 0x15;
    rect.right = rect.right + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(2);
        return;
    }

    rect.right = rect.right + 0x15;
    rect.left = rect.left + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(1);
        return;
    }

    rect.left = rect.left + 0x15;
    rect.right = rect.right + 0x15;
    if (in_box(x_coord, y_coord, &rect)) {
        reinterpret_cast<VCallI *>(
            entered + *reinterpret_cast<int32_t *>(*reinterpret_cast<int32_t *>(entered - 0x21A6C) + 4) -
            0x21a6c)
            ->slot044(0);
    }
}

// ORIGINAL: 0x0046FD90 ?clear_terrain@MapWin@@QAEXXZ 0x0046FD90-0x0046FDFE FILE
// size      110 bytes
// prototype void (__thiscall ?clear_terrain@MapWin@@QAEXXZ)(MapWin* this)
// callers   4   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
// Both passes load the owned pointer directly from MapWin+4. Clear every
// brightness record before starting the altitude pass; bytes 9..11 retain
// their terrain/detail state. A nonpositive signed MapArea does no writes.

void MapWin::clear_terrain() {
    if (owned_ != 0) {
        for (int entry = 0; entry < static_cast<int32_t>(MapArea); ++entry) {
            for (int corner = 0; corner < 4; ++corner) {
                owned_[entry].brightness[corner] = 0xFF;
            }
        }
        if (owned_ != 0) {
            for (int altitude_entry = 0; altitude_entry < static_cast<int32_t>(MapArea);
                 ++altitude_entry) {
                for (int slot = 0; slot < 5; ++slot) {
                    owned_[altitude_entry].altitude[slot] = -1;
                }
            }
        }
    }
}

// Compiler-generated Win-branch capture adjustor from the genuine
// MapWin override and virtual GraphicWin base; no handwritten thunk body.
/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_lose_mouse_capture expects, then forward unchanged.
// ORIGINAL: 0x00421870 ?on_lose_mouse_capture@thunk1_MapWin@@QAEXXZ 0x00421870-0x00421878 FILE BYTE_EXACT
// symbol    ?on_lose_mouse_capture@MapWin@@$4PPPPPPPM@A@AEXXZ
// size      8 bytes
// prototype void (__thiscall ?on_lose_mouse_capture@thunk1_MapWin@@QAEXXZ)(MapWin* this)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot13 adjustor for the genuine MapWin resize
// override. The virtual-base layout owns the receiver adjustment.
/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_resize expects, then forward unchanged.
// ORIGINAL: 0x00421890 ?on_resize@thunk1_MapWin@@QAEXHH@Z 0x00421890-0x00421898 FILE BYTE_EXACT
// symbol    ?on_resize@MapWin@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_resize@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// Compiler-generated Win slot17 receiver adjustment for MapWin mouse feedback.
// The original catalogue name remains on the mapping below.
/*
Purpose: Adjust the receiver from MapWin's thunk1 subobject to the one
         MapWin::on_mouse_move expects, then forward unchanged.
// ORIGINAL: 0x004218B0 ?on_mouse_move@thunk1_MapWin@@QAEXHH@Z 0x004218B0-0x004218B8 FILE BYTE_EXACT
// symbol    ?on_mouse_move@MapWin@@$4PPPPPPPM@A@AEXHH@Z
// size      8 bytes
// prototype void (__thiscall ?on_mouse_move@thunk1_MapWin@@QAEXHH@Z)(MapWin* this, int, int)
// callers   0   call targets   0
// kind      game
// flags     hidden;sp_ready;purged_ok
// calls     (none)
Return Value: n/a
Status: Complete
*/

// The C++ compiler owns the scalar deleting destructor below. Do not
// add a hand-written deleting wrapper or explicit destructor cascade.
// Current source ownership is not original TU or COMDAT-selection proof.
// ORIGINAL: 0x00421830 ??_GMapWin@@UAEPAXI@Z 0x00421830-0x00421866 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["historical seed:src/recovered/units/00421830.cpp", "task:window-generated-deletion-owners", "Pinned entry receives the MapWin virtual GraphicWin subobject at complete-object+0x21A6C; own-part destructor, virtual-base destructor and conditional delete are separate native stages.", "Fresh emitted code and receiver/target controls remain required. Duplicate COMDAT emission does not establish historical translation-unit placement or retained link selection."], "reason": "The existing real class lifetime generates this deleting entry; bind compiler-owned code rather than transcribing receiver arithmetic or invoking member/base destructors twice."}
