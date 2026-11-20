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
#include "xpops.h"
#include "popup.h"   // PopupStartCaption, the shared caption buffer
#include "faction.h"  // PopupDialogFactionID, ExpansionEnabled
#include <cstdio>
#include <cstring>
#include "general.h"
#include "strings.h"
#include "text.h"
#include "game.h"
#include "console.h"
#include <shellapi.h>
#include <ctype.h> // yearmotize preserves the original signed-byte CRT arguments.
#include <stddef.h>
#include <mmsystem.h>
#include "win.h"
#include "ambience.h"  // load_music, the startup faction ambience owner.
#include "interface_palette.h"
#include "sound.h"
#include "netmsg.h"
#include "alpha.h"
#include "base.h"
#include "basepop.h"  // BasePopDefaultField322C
#include "buffer.h"  // g_PCX_PARSE_TEMP_BUFFER1
#include "graphicwin.h"  // opening window's Buffer base and virtual update
#include "sprite.h"  // Sprite::extract
#include "texture.h" // Terrain-sheet texture assets.
#include "basewin.h"  // BaseWin::close
#include "designwin.h"  // DesignWin::close
#include "fx.h"  // g_FX
#include "filebox.h"
#include "prefwin.h"  // ConsolePrefWin.init needs the complete owner declaration.
#include "lock.h"
#include "map.h"
#include "main.h"  // CommandLineText
#include "mapwin.h" // draw_tile, mapwin_system_shutdown
#include "multiwin.h"
#include "log.h"  // log_set_state
#include "random.h"  // random_reseed
#include "battlewin.h"  // the standing battle window
#include "datalink.h"  // DatalinkMain
#include "messagewin.h"  // the standing MessageWin
#include "multidebug.h"  // MultiDebug, the standing debug window
#include "netdaemon.h"  // NetDaemonNet
#include "halloffame.h"
#include "interlude.h"
#include "quaylewin.h"
#include "reportif.h"
#include "statuswin.h"  // StatusWin::close
#include "time.h"  // stop_timers
#include "tutwin.h"  // TutWinDesWindow, TutWinBaseWindow, TutWinIfaceWindow
#include "veh.h"
#include "crt_random_inline.h"  // bounded_crt_random
#include "wave.h"  // g_WAVE_GENERAL
#include "worldwin.h"  // WorldClimateWorldWin
#include "diplowin.h"  // DiploWin::exec, homed here out of the archive
#include "diplopop.h"  // DiplomacyPopup
#include "filemap.h"
#include "palette.h"
#include "caviar.h"
#include "vector.h"  // free three-float operations
#include "newtechwin.h"  // Existing technology-popup sprite assets.
#include "infowin.h"  // Shared faction text/shadow colors.
#include "maininterface.h"  // Shared chat foreground/shadow colors.
#include "pushbutton.h"
#include "flatbutton.h"
#include "council.h"
#include "councwin.h"  // game_data transfers the live council window fields
#include "replaywin.h"
#include "technology.h"
#include "path.h"
#include "alphasave.h"
#include "netwin.h"
#include "deletionlist.h"
#include "gamma.h"
#include "map_coordinates.h"
#include "terraforming.h"
#include "net_checksum_report.h"
#include "flic.h"
#include "font.h"
#include "font_queue.h"

// The game state is defined by this unit (MATCHING_GUIDE 2.5, data order).
// .data: the initialised globals 0x00691870-0x00691B3C and the texts of their
// initialisers 0x00691B40-0x00691BF7 lie after technology's last function
// literal (0x0069185C) and run straight into this unit's first function
// literal (0x00691BF8, do_unity_crash): a unit's globals precede its function
// literals. .bss: this unit's two initializer thunks build g_PALETTE1
// 0x0094C590 and g_PALETTE2 0x009523A0, so the run around them, after
// technology's objects (..0x00945F44) and before veh_queries' (0x009B2278..),
// is this unit's. TurnUpkeepPhase 0x00945F48 touches technology's last object
// and could be either unit's (a unit's .bss is ordered by name, not by
// definition). Definitions below are in image order, initialised first.

// The unit's leading pointer globals are initialised with literals: the image
// holds the pointer cells among the globals (0x00691870..0x00691B3C) and
// their texts as one block behind the last global, in the same order
// (0x00691B40..0x00691BF7), and nothing but its cell references a text. The
// cell at 0x00691B10 (-> "EDITOR", 0x00691B84) has no reader in the image and
// no definition here.
// RECOVERY_OBJECT: v2 0x00691870 0x00691874 ?ConsoleVersionRelease@@3PADA 0x00517180
// RECOVERY_LITERAL: v1 0x00691B40 0x00691B4C $SG65273 0x00517180
char *ConsoleVersionRelease = "RELEASE 2.0";
// RECOVERY_OBJECT: v2 0x00691874 0x00691878 ?ConsoleVersionBuildDate@@3PADA 0x00517180
// RECOVERY_LITERAL: v1 0x00691B4C 0x00691B58 $SG65275 0x00517180
char *ConsoleVersionBuildDate = "20-Dec-1999";
// RECOVERY_OBJECT: v2 0x00691878 0x00691AF8 ?Terraforming@@3PAURulesTerraforming@@A 0x005873C0
// The historical map word identities are live rates in this mutable array.
// 0x00691890 + row * 32 selects rate (+0x18), including road and magtube.
// RECOVERY_OBJECT: v3 0x00691930 ?MapTerrainBonusYieldRow5FirstWord@@3HB member ?Terraforming@@3PAURulesTerraforming@@A [5].rate
// RECOVERY_OBJECT: v3 0x00691950 ?MapTerrainBonusYieldRow6FirstWord@@3HB member ?Terraforming@@3PAURulesTerraforming@@A [6].rate
RulesTerraforming Terraforming[MaxTerrainNum] = {
    { 0, 0, {0, 0}, BIT_FARM,          BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_SOIL_ENRICHER, BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_MINE,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_SOLAR_TIDAL,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_FOREST,
      BIT_MINE | BIT_FUNGUS | BIT_SOLAR_TIDAL | BIT_FARM | BIT_SOIL_ENRICHER
          | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR | BIT_THERMAL_BORE, 0, 0 },
    { 0, 0, {0, 0}, BIT_ROAD, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_MAGTUBE, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_BUNKER, BIT_AIRBASE, 0, 0 },
    { 0, 0, {0, 0}, BIT_AIRBASE, BIT_BUNKER, 0, 0 },
    { 0, 0, {0, 0}, BIT_SENSOR_ARRAY,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE, 0, 0 },
    { 0, 0, {0, 0}, 0, BIT_FUNGUS, 0, 0 },
    { 0, 0, {0, 0}, BIT_FUNGUS,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FARM | BIT_SOIL_ENRICHER | BIT_FOREST, 0, 0 },
    { 0, 0, {0, 0}, BIT_CONDENSER,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_ECH_MIRROR,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FOREST | BIT_CONDENSER | BIT_ECH_MIRROR
          | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, BIT_THERMAL_BORE,
      BIT_MINE | BIT_SOLAR_TIDAL | BIT_FARM | BIT_FOREST | BIT_CONDENSER
          | BIT_ECH_MIRROR | BIT_THERMAL_BORE | BIT_SENSOR_ARRAY, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, 0, 0, 0, 0 },
    { 0, 0, {0, 0}, BIT_MONOLITH,
      BIT_SUPPLY_POD | BIT_NUTRIENT_RSC | BIT_RSC_BONUS, 0, 0 },
}; // 0x00691878
// The tile-flag mask the fungus pass clears off the Map record it has just
// given bit 0x20 to: world_fungus does `tile->bit &= ~MapTileFlagClearMask`
// (0x005C35EB, `not edx` then `and [ecx + eax*4 + 8], edx`), and the same
// dword is the value argument at four more sites that set tiles through
// 0x00591D60 (0x004CE6D5 is one). Eight readers in the whole image, all of
// them reads, and the shipped .data holds 0x00288050 - the same constant the
// 0x004CE6C9 call site pushes as a literal plus 0xc. No image symbol and no
// name anywhere in the tree before this.
// Start binding only: the original DWORD reads establish the accessed value,
// not an enclosing allocation or the absence of indirect writers.
// RECOVERY_OBJECT: v1 0x006919EC ?MapTileFlagClearMask@@3IA
uint32_t MapTileFlagClearMask = 0x00288050;
// The original show/hide pair publishes its visible state in
// MainInterfaceGlobal.multiwin_visibility_state_ (0x007CFF04). Its historical
// object identity is retained beside MainInterfaceGlobal in maininterface.cpp.

// Shared animation frame interval (20 ms in the pinned image). Other window
// animations also read this original data object.
// RECOVERY_OBJECT: v2 0x00691AF8 0x00691AFC ?WindowAnimationFrameDelay@@3HA 0x00479C00
int WindowAnimationFrameDelay = 20;
// read_tech's prologue loads the pointer variable (`mov eax, dword ptr
// [0x691AFC]` at 0x00585E34) ahead of text_open; the DWORD there points at
// the "ALPHAX" literal at 0x00691B58.
// RECOVERY_OBJECT: v2 0x00691AFC 0x00691B00 ?AlphaxFileID@@3PBDB 0x0048207D
// The initialiser's own literal: the image DWORD at 0x00691AFC holds
// 0x00691B58, where the seven bytes "ALPHAX\0" sit.
// RECOVERY_LITERAL: v1 0x00691B58 0x00691B5F $SG65277 0x0048207D
LPCSTR AlphaxFileID = "ALPHAX";
// RECOVERY_OBJECT: v2 0x00691B00 0x00691B04 ?DatalinkHelpFile@@3PADA 0x00430770
// RECOVERY_LITERAL: v1 0x00691B60 0x00691B66 $SG65278 0x00430770
char *DatalinkHelpFile = "HELPX";
// RECOVERY_OBJECT: v2 0x00691B04 0x00691B08 ?DatalinkBlurbFile@@3PADA 0x00430B30
// RECOVERY_LITERAL: v1 0x00691B68 0x00691B70 $SG65280 0x00430B30
char *DatalinkBlurbFile = "BLURBSX";
// Shared storage closure belongs to the coordinator's original owners.
// The pointer cell selects this observed NUL-terminated text-file identifier.
// The allocation extent of the pointed-to original string is unresolved.
// RECOVERY_OBJECT: v1 0x00691B08 ?InterludeTextFile@@3PADA
// RECOVERY_LITERAL: v1 0x00691B70 0x00691B7B $SG65282 0x00459680
char *InterludeTextFile = "INTERLUDEX";
// Original691B0C contains pointer691B7C, whose file-backed text is SCRIPT.
// The original bytes are "SCRIPT" and its NUL; one zero pad byte precedes the next string, "EDITOR", at 0x00691B84.
// Bound: pointer cell; the next dword 0x00691B10 is a separate pointer (to "EDITOR" at 0x00691B84).
// RECOVERY_OBJECT: v2 0x00691B0C 0x00691B10 ?ScriptPopupFile@@3PADA 0x00406384
// RECOVERY_LITERAL: v1 0x00691B7C 0x00691B83 $SG65283 0x00406384
char *ScriptPopupFile = "SCRIPT";
// Shared tutorial popup caption: the original pointer cell selects the
// six-byte TUTOR string, not the adjacent WIZARD caption.
// RECOVERY_OBJECT: v2 0x00691B14 0x00691B18 ?TutorialPopupCaption@@3PADA 0x00511800
// RECOVERY_LITERAL: v1 0x00691B8C 0x00691B92 $SG65285 0x00511800
char *TutorialPopupCaption = "TUTOR";
// The published unit draw-state block: the words the surface sprite's
// draw_dest_unk1 readers consume, 0x009B3A54..0x009B3A78.


// ---------------------------------------------------------------------------
// FX::play, 0x00446A00. The bank's sound trigger: gate on the sound bit of
// GamePreferences, re-arm the popup wave owner for the first three effect
// kinds, then release/restart the effect record. fx.h declares the member;
// its forwarder retires from pending_bodies.cpp in this change.

// Interlude members and compiler-generated lifetime code are owned by interlude.cpp.

// Media overlay update is owned by MediaRuntime in movie_media.cpp.


// Original pointer cell 0x00691B18 selects the WIZARD text at 0x00691B94.
// RECOVERY_OBJECT: v1 0x00691B18 ?TechWizardPopupFile@@3PADA
// RECOVERY_LITERAL: v1 0x00691B94 0x00691B9B $SG65287 0x005BDE20
char *TechWizardPopupFile = "WIZARD";
// Original pointer cell 0x00691B1C selects the MENU script name at
// 0x00691B9C; menu_init passes the cell's value to every text_open.
// RECOVERY_OBJECT: v1 0x00691B1C ?MenuScriptFile@@3PADA
// RECOVERY_LITERAL: v1 0x00691B9C 0x00691BA1 $SG65289 0x0045FB20
char *MenuScriptFile = "MENU";
// Start's mod-redirection data (popup.h has the role note): the image's
// pointer slots select these NUL-terminated text identifiers.
// RECOVERY_OBJECT: v1 0x00691B20 ?PopupStartAltFile1@@3PADA
// RECOVERY_LITERAL: v1 0x00691BA4 0x00691BB1 $SG65290 0x00406380
char *PopupStartAltFile1 = "alienIscript";    // 0x00691B20
// RECOVERY_OBJECT: v1 0x00691B24 ?PopupStartAltFile2@@3PADA
// RECOVERY_LITERAL: v1 0x00691BB4 0x00691BC1 $SG65291 0x00406380
char *PopupStartAltFile2 = "alienuscript";    // 0x00691B24
// Application fonts, selected and initialized by config_popups and WinMain.
// FontTable stays with find_font; the entrypoint's singleton is in main.cpp.
// Original CRT order is singleton, FontQueue VAL2/VAL1, then FontTable.

// Face pointer read only by Credits::exec (0x00428AE4); its literal sits
// before "Arial" in this unit's data.
// RECOVERY_OBJECT: v1 0x00691B28 ?CreditsFontFace@@3PBDB
// RECOVERY_LITERAL: v1 0x00691BC4 0x00691BD3 $SG65293 0x004289F0
LPCSTR CreditsFontFace = "Alpha Centauri";  // 0x00691B28 -> 0x00691BC4
// RECOVERY_OBJECT: v2 0x00691B2C 0x00691B30 ?DefaultFontFace@@3PBDB 0x00408B8D
// RECOVERY_LITERAL: v1 0x00691BD4 0x00691BDA $SG65295 0x00408B8D
LPCSTR DefaultFontFace = "Arial";  // 0x00691B2C -> 0x00691BD4
// RECOVERY_OBJECT: v1 0x00691B30 ?DefaultLargeFontFace@@3PBDB
// RECOVERY_LITERAL: v1 0x00691BDC 0x00691BE9 $SG65297 0x00427230
LPCSTR DefaultLargeFontFace = "Arial Narrow"; // 0x00691B30 -> 0x00691BDC
// The original 0x691B34 pointer cell refers to the text at 0x691BEC.
// RECOVERY_OBJECT: v1 0x00691B34 ?SaveGameFileExtension@@3PBDB
// RECOVERY_LITERAL: v1 0x00691BEC 0x00691BF0 $SG65298 0x005A94F0
LPCSTR SaveGameFileExtension = "SAV";
// Native scenario save/load uses pointerB38 toSC; map save/load uses
// pointerB3C toMP.
// RECOVERY_OBJECT: v1 0x00691B38 ?ScenarioFileExtension@@3PBDB
// RECOVERY_LITERAL: v1 0x00691BF0 0x00691BF3 $SG65299 0x005A9EB0
LPCSTR ScenarioFileExtension = "SC";
// Historical SC attribution of this cell was wrong; originalB3C points toMP.
// RECOVERY_OBJECT: v1 0x00691B3C ?MapFileExtension@@3PBDB
// RECOVERY_LITERAL: v1 0x00691BF4 0x00691BF7 $SG65300 0x00590ED0
LPCSTR MapFileExtension = "MP";
// net_upkeep5290D0/5292BD publish phases1/2; tech_advance5BE580 and
// tech_research5BEB3A read this distinct shared dword. Original virtual-tail
// storage is loader-zero; fresh compiler objects establish one int owner.
// Remaining net_upkeep recovery must publish to this owner, not an old address.
// RECOVERY_OBJECT: v2 0x00945F48 0x00945F4C ?TurnUpkeepPhase@@3HA 0x005BE940
int TurnUpkeepPhase = 0;
// RECOVERY_OBJECT: v2 0x00946020 0x00946138 ?Citizen@@3PAURulesCitizen@@A 0x005873C0
RulesCitizen Citizen[MaxCitizenNum];                // 0x00946020
// Original interlude reads/writes this complete story-mask word.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Wire0 and game_data transfer these exact independent byte intervals.
// No direct semantic use or original allocation boundary is proved; they are
// retained state, never padding or zero-producing packet filler. Keep the
// filename tail separate from the exactly bounded legacy 256-byte copy.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Native game_data5A6BA3..5A6BB9 copies exactly256 bytes; setup_game
// clears only byte0. NUL-scanning consumers do not prove loaded termination.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// Logical faction0..7 indexing is independently observed; original
// containing allocation bounds are not asserted.
// RECOVERY_OBJECT: v1 0x00946138 ?TectonicDetonationCount@@3PAHA
int TectonicDetonationCount[8];
// Convoy 004D907E/004D90EF and the rules parser independently identify this
// existing resource-table start; lifetime remains open.
// Bound: the loader fills 4 x 8-byte records from +4 to `cmp esi, 0x94617c` (0x00587D75).
// RECOVERY_OBJECT: v2 0x00946158 0x00946178 ?Resource@@3PAURulesResource@@A 0x00587D75
RulesResource Resource[MaxResourceNum];             // 0x00946158
// RECOVERY_OBJECT: v1 0x00946580 ?SocialEffects@@3PAURulesSocialEffect@@A
RulesSocialEffect SocialEffects[MaxSocialEffectNum];        // 0x00946580
// RECOVERY_OBJECT: v2 0x009469F8 0x009469FC ?ReplayEventBytes@@3HA 0x005ADE80
int ReplayEventBytes;
// RECOVERY_OBJECT: v1 0x009469FC ?PopupModalActive@@3HA
int PopupModalActive;  // 0x009469FC
// read_faction(int) (0x00586050) computes its Player* argument as
// `lea ecx, [eax*4 + 0x946A50]` (stride 1436); faction.h repeats the base
// beside its extern declaration.
// Bound: 8 x 0x59C records; loops end at `cmp esi, 0x949730` (0x00424A96), RulesStorage's start.
// RECOVERY_OBJECT: v2 0x00946A50 0x00949730 ?Players@@3PAUPlayer@@A 0x00424A96
Player Players[MaxPlayerNum];                               // 0x00946A50
// NetDaemon::synch sizes opcode 0x81, this whole window, as 0x13C bytes
// (`mov esi, 0x13c` at 0x00533226; `mov esi, 0x949730` at 0x00533A5C).
// RECOVERY_OBJECT: v2 0x00949730 0x0094986C ?RulesStorage@@3URulesStorageWindow@@A 0x00533226
// RECOVERY_OBJECT: v3 0x00949738 ?Rules@@3URulesBasic@@A member ?RulesStorage@@3URulesStorageWindow@@A rules_
// RECOVERY_OBJECT: v3 0x00949730 ?TechValidCount@@3HA member ?RulesStorage@@3URulesStorageWindow@@A tech_valid_count_
// RECOVERY_OBJECT: v3 0x00949734 ?TechCommerceCount@@3HA member ?RulesStorage@@3URulesStorageWindow@@A tech_commerce_count_
RulesStorageWindow RulesStorage;
// Metadata scalars formerly lived here; landmarks_ and file_path_ formerly
// lived in alpha.cpp. All had static zero initialization. Their exact native
// address/offset relationship is now declared in map.h in one real allocation.
// MapLongitude (68FAF0) remains an independently initialized row-stride object.
// map_read/map_write transfer this writable zero-fill window as 0xAA4 bytes.
// This records the observed serialized region, not a claim about original type,
// complete allocation, or lifetime.
// RECOVERY_OBJECT: v2 0x00949870 0x0094A314 ?MapStorage@@3UMapStorageWindow@@A 0x005910B0
// RECOVERY_OBJECT: v3 0x0094A30C ?MapTiles@@3PAUMap@@A member ?MapStorage@@3UMapStorageWindow@@A tiles_
// RECOVERY_OBJECT: v3 0x0094A310 ?MapAbstractTiles@@3PAEA member ?MapStorage@@3UMapStorageWindow@@A abstract_tiles_
// RECOVERY_OBJECT: v3 0x00949870 ?MapLongitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.longitude_bounds_
// RECOVERY_OBJECT: v3 0x00949874 ?MapLatitudeBounds@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.latitude_bounds_
// RECOVERY_OBJECT: v3 0x00949884 ?MapArea@@3IA member ?MapStorage@@3UMapStorageWindow@@A metadata_.area_
// RECOVERY_OBJECT: v3 0x0094988C ?MapIsFlat@@3HA member ?MapStorage@@3UMapStorageWindow@@A metadata_.is_flat_
MapStorageWindow MapStorage;
// The rules loader fills entries0..8; original load_vehicles45446C..4546B4
// additionally populates sprite metrics in entries9..18 at the same0x90 stride.
// The full table ends at Natural's independent start0x0094ADE0.
// RECOVERY_OBJECT: v2 0x0094A330 0x0094ADE0 ?Chassis@@3PAURulesChassis@@A 0x00454270
RulesChassis Chassis[ChassisStorageCount];  // 0x0094A330
// The original loader at 0x5881AB starts at 0x94ADE4 and fills name/name_short
// at offsets -4/0 with stride8. Terrain consumers read those same pointer slots.
// Lifetime remains open.
// Bound: 16 x 8-byte records; loop ends at `cmp esi, 0x94ae60` (0x004E155B), Weapon's start.
// RECOVERY_OBJECT: v2 0x0094ADE0 0x0094AE60 ?Natural@@3PAURulesNatural@@A 0x004E155B
RulesNatural Natural[MaxNaturalNum];                        // 0x0094ADE0
// Bound: 26 x 0x10 records; loop ends at `cmp esi, 0x94b000` (0x0044BE13), SocialCategories' start.
// RECOVERY_OBJECT: v2 0x0094AE60 0x0094B000 ?Weapon@@3PAURulesWeapon@@A 0x0044BE13
RulesWeapon Weapon[MaxWeaponNum];  // 0x0094AE60
// THE FACTION/MAP RULES STORAGE (faction.h / map.h) - real zero-initialised
// objects; the image's copies are uninitialised .data the rules walkers fill.
// Original say_society at 0x00590A21 reads the category-row start at stride 0xD4; no full layout/extent claim.
// RECOVERY_OBJECT: v1 0x0094B000 ?SocialCategories@@3PAURulesSocialCategory@@A
RulesSocialCategory SocialCategories[MaxSocialCatNum];      // 0x0094B000
// Extent: compiled sizeof 0x14; its last field top_menu (0x0094B360) is
// stored by prefs_load at 0x0059DE8E and read by map_menu/top_menu.
// RECOVERY_OBJECT: v2 0x0094B350 0x0094B364 ?DefaultPrefs@@3UDefaultPref@@A 0x0059DE8E
DefaultPref DefaultPrefs;                           // 0x0094B350
// prefs_load (0x0059DCF0) stores every field in place - `mov dword ptr
// [0x94B464], eax` for preferences through custom_world at 0x0094B480
// (bounded by the `cmp esi, 0x94B49C` loop tail at 0x0059E4D9).
// Extent: compiled sizeof 0x38; prefs_load loop ends at `cmp esi, 0x94B49C` (0x0059E4D9).
// RECOVERY_OBJECT: v2 0x0094B464 0x0094B49C ?AlphaIniPrefs@@3UAlphaIniPref@@A 0x0047D4A4
AlphaIniPref AlphaIniPrefs;                         // 0x0094B464
// RECOVERY_OBJECT: v2 0x0094B4A0 0x0094B4C0 ?Mandate@@3PAURulesMandate@@A 0x005873C0
RulesMandate Mandate[MaxMandateNum];                // 0x0094B4A0
// 0x009A6494
// 0x009A6498
// 0x009A649C
// 0x009A64C0
// 0x009A64C4
// 0x009A64D4
// setup_game clears this independent DWORD after BaseCurrentCount. No
// additional semantic use is established; it is not part of the base count.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// set_time_controls publishes its selected seconds multiplied by 1000.
// net_control_turn copies the duration to the distinct live wait accumulator.
// Historical object identity moved to GameDataStateStorage; see the transport manifest.

// game_data transfers 0x98 bytes from 0x0094B4C0 (5A8337); Mandate ends at its
// start and the next referenced storage is 0x0094B558.
// RECOVERY_OBJECT: v2 0x0094B4C0 0x0094B558 ?ScenarioSave@@3UScenarioSaveState@@A 0x005A6AC0
// RECOVERY_OBJECT: v3 0x0094B4D8 0x0094B558 ?GameWireOpaque_94B4D8@@3PAEA 0x005A6AC0 member ?ScenarioSave@@3UScenarioSaveState@@A wire_opaque_94b4d8
ScenarioSaveState ScenarioSave;
// The faction prototype-design list (ProtoBung, declared in veh.h). Eight
// faction histories, 64 records each. Initial loaded storage is zero.
// Native clear57DF00 uses base94B558+5 then stores at -5..+2; the former
// address alias started five bytes too early and corrupted the preceding data.
// RECOVERY_OBJECT: v2 0x0094B558 0x0094C558 _BungList 0x0057DF00
ProtoBung BungList[8][64];
// Eight faction blocks begin at94CA08, with21records at38 stride and
// latest-type at+498. Founding/secrets/victory consumers establish the
// natural container; reset's previous94CA18 view began inside record0.
// RECOVERY_OBJECT: v2 0x0094CA08 0x0094F1A8 ?FactionMonuments@@3PAUMonumentFaction@@A 0x00476DA0
MonumentFaction FactionMonuments[MaxPlayerNum];
// RECOVERY_OBJECT: v1 0x0094F1B8 ?TimeControl@@3PAURulesTimeControl@@A
RulesTimeControl TimeControl[MaxTimeControlNum];    // 0x0094F1B8
// Bound: 14 x 0x10 records; loop ends at `cmp esi, 0x94f358` (0x0044BFE0), Technology's start.
// RECOVERY_OBJECT: v2 0x0094F278 0x0094F358 ?Armor@@3PAURulesArmor@@A 0x0044BFE0
RulesArmor Armor[MaxArmorNum];  // 0x0094F278
// MapFilePath at 0x0094A2BC is MapStorage.metadata_.file_path_ (map.cpp).
// read_tech walks the array from `mov esi, 0x94F360` (&Technology[0].id,
// base+8 over flags/name) with the 0x2C stride declared in technology.h.
// Bound: 89 x 0x2C records; loops end at `cmp esi, 0x9502a4` (0x0059F7C0).
// RECOVERY_OBJECT: v2 0x0094F358 0x009502A4 ?Technology@@3PAURulesTechnology@@A 0x0059F7C0
RulesTechnology Technology[MaxTechnologyNum];       // 0x0094F358
// The parser walks the five ratio words at +0x64..+0x74. NetDaemon::synch
// opcode 0x82 transfers 0x7C bytes from this start (0x00533230/0x00533A63).
// That transfer size bounds the object; ReplayEvents starts at its end.
// RECOVERY_OBJECT: v2 0x009502A8 0x00950324 ?WorldBuilder@@3URulesWorldbuilder@@A 0x00533230
RulesWorldbuilder WorldBuilder;
// setup_game clears 0x800 dwords at 0x00950324 and timer_callback reads
// eight-byte records from the same interval. The counter advances in bytes.
// RECOVERY_OBJECT: v2 0x00950324 0x00952324 ?ReplayEvents@@3PADA 0x005ADE80
char ReplayEvents[0x2000];
// RECOVERY_OBJECT: v1 0x00952328 ?Morale@@3PAURulesMorale@@A
RulesMorale Morale[MaxMoraleNum];  // 0x00952328
// The original four 0xC-byte reactor records directly precede the 0x34-byte
// vehicle table. Keep these independent, zero-initialized arrays together;
// their original translation-unit name and any shared class remain unknown.
// The rules loader supplies reactor fields before use; vehicles are populated
// by their existing construction and game-loading owners.
// Original 0x0042A910 selects the record base; best_reactor walks prerequisite fields.
// Bound: 4 x 0xC records; loop ends at `cmp esi, 0x952828` (0x0044BAD6), Vehs' start.
// RECOVERY_OBJECT: v2 0x009527F8 0x00952828 ?Reactor@@3PAURulesReactor@@A 0x0044BAD6
RulesReactor Reactor[MaxReactorNum];  // 0x009527F8
// 2049 0x34-byte slots (veh_fake uses id 2048) end at 0x0096C85C, which the
// image indexes as a separate table base (`[edx*4+0x96c85c]` at 0x00449978).
// RECOVERY_OBJECT: v2 0x00952828 0x0096C85C ?Vehs@@3PAUVeh@@A 0x005C1D50
Veh Vehs[2049];  // 0x00952828
// 30 x 0xC records; PlayersData begins at the bound. 0x004316FD reads Order[4].order.
// RECOVERY_OBJECT: v2 0x0096C878 0x0096C9E0 ?Order@@3PAURulesOrder@@A 0x004316FD
RulesOrder Order[MaxOrderNum];  // 0x0096C878
// Eight 0x20CC-byte records end where Bases begins at 0x0097D040.
// RECOVERY_OBJECT: v2 0x0096C9E0 0x0097D040 ?PlayersData@@3PAUPlayerData@@A 0x00458DC0
PlayerData PlayersData[MaxPlayerNum];                       // 0x0096C9E0
// The pinned add-base path rejects when BaseCurrentCount reaches 512
// (0x004E4B80); Base is 0x134 bytes, so the bounded array ends at 0x009A3840.
// RECOVERY_OBJECT: v2 0x0097D040 0x009A3840 ?Bases@@3PAUBase@@A 0x004E4B80
Base Bases[MaxBaseNum];                             // 0x0097D040
// hof_save/hof_load transfer this complete zero-initialized 0x1324-byte
// block, and FameWin indexes it in 0x8C-byte records. The original address
// is evidence of identity, not a pointer used by the product build.
// RECOVERY_OBJECT: v2 0x009A3840 0x009A4B64 ?HallOfFameRecords@@3PAUHallOfFameRecord@@A 0x0044AF70
HallOfFameRecord HallOfFameRecords[HallOfFameRecordCount];  // 0x009A3840
// Secret projects are the final 64 records, beginning at Facility[70]
// (original 0x009A5888). The parser and consumers share this real array;
// there is no separately allocated secret-project table.
// Bound: 134 x 0x30 records; loops end at `cmp edi, 0x9a6488` (0x0048553E).
// RECOVERY_OBJECT: v2 0x009A4B68 0x009A6488 ?Facility@@3PAURulesFacility@@A 0x0048553E
RulesFacility Facility[MaxFacilityNum];             // 0x009A4B68
// The rainfall loader publishes three rows of four colors, one byte after
// each get_pixel. The coast renderer consumes row 1 forward or backward.

// Original 452258 walks exactly 53 palette indices. No writer or pointer
// escape was observed; const is that bounded interpretation, not a claim
// about arbitrary computed addresses. Irregular image data stays in the
// pinned, ignored configure-time include, never a second source ledger.


// Mutable panel styles: load_box_sprites fills three columns by seventeen
// rows. The records borrow the actual Sprite and Buffer objects below.
// Original 0x78D528..0x78DD20 is initially zero; it is not a static address table.
// The original loader advances 0x28 per record and stops after 51 buffers.


// Superseded seed deleted; seed paths below are historical Git provenance.
// Each row maps every palette index to its row number (original 0x007A81A0).

// Original 0x006861F0..0x00686330: mutable palette offsets selected by
// alt_get_ocean_detail's 0..79 result. Configuration replaces only the
// accepted prefix, so these startup values and later untouched entries
// persist across missing, partial and repeated load_cycling calls.

// The temporary model retains the fourth record until scope exit, including
// unwinding. A valid record with a different step count leaves its offset alone.



// Bound: scalar; every original access is a dword at the start, never indexed, and the following bytes are never referenced.
// RECOVERY_OBJECT: v2 0x009A6488 0x009A648C ?ExpansionEnabled@@3HA 0x004063FB
BOOL ExpansionEnabled;  // 0x009A6488
// RECOVERY_OBJECT: v3 0x009A64EC 0x009A650C ?FactionRankings@@3PAIA 0x005AC690 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.faction_rankings_
// RECOVERY_OBJECT: v3 0x009A6514 0x009A6614 ?SecretProject@@3UBaseSecretProject@@A 0x005002E0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.secret_project_
// RECOVERY_OBJECT: v3 0x009A64CC 0x009A64D0 ?BaseCurrentCount@@3HA 0x004E68B0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.base_current_count_
// RECOVERY_OBJECT: v3 0x009A6614 0x009A6618 ?GovernorFaction@@3HA 0x00424410 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.governor_faction_
// RECOVERY_OBJECT: v3 0x009A67DC 0x009A67E0 ?TranscendenceScoreBonus@@3HA 0x005ACBE0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.transcendence_score_bonus_
// RECOVERY_OBJECT: v3 0x009A67E0 0x009A6800 ?TranscendenceScoreWeights@@3PAHA 0x005ACBE0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.transcendence_score_weights_
// RECOVERY_OBJECT: v3 0x009A64E0 ?TimeControlsSelection@@3HA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.time_controls_selection_
// RECOVERY_OBJECT: v3 0x009A6618 0x009A6644 ?CouncilResolutionCount@@3PAHA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.council_resolution_count_
// RECOVERY_OBJECT: v3 0x009A6644 0x009A6670 ?CouncilLastResolutionTurn@@3PAHA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.council_last_resolution_turn_
// RECOVERY_OBJECT: v3 0x009A64D0 0x009A64D4 ?GameSetupResetWord@@3HA 0x005B3920 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_setup_reset_word_
// RECOVERY_OBJECT: v3 0x009A64E4 0x009A64E8 ?TurnControlDurationMilliseconds@@3HA 0x00523C60 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.turn_control_duration_milliseconds_
// RECOVERY_OBJECT: v3 0x009A6810 0x009A6814 ?SaveDaemonStateWord@@3HA 0x005A94F0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.save_daemon_state_word_
// RECOVERY_OBJECT: v3 0x009A6824 0x009A6828 ?EcologyEventAccumulator@@3HA 0x004F67F0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.ecology_event_accumulator_
// RECOVERY_OBJECT: v3 0x009A64A4 ?GameInterludeFlags@@3IA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_interlude_flags_
// RECOVERY_OBJECT: v3 0x009A64A0 0x009A64A4 ?GameWireOpaque_9A64A0@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a64a0_
// RECOVERY_OBJECT: v3 0x009A64A8 0x009A64AC ?GameWireOpaque_9A64A8@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a64a8_
// RECOVERY_OBJECT: v3 0x009A66C9 0x009A67C9 ?LegacyGameSaveName@@3PADA 0x005A6AC0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.legacy_game_save_name_
// RECOVERY_OBJECT: v3 0x009A67C9 0x009A67CC ?GameWireOpaque_9A67C9@@3PAEA 0x00532E00 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_wire_opaque_9a67c9_
// RECOVERY_OBJECT: v3 0x009A67CC 0x009A67D0 ?ClimateRiseStep@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_step_
// RECOVERY_OBJECT: v3 0x009A67D0 0x009A67D4 ?ClimateRiseLimit@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_limit_
// RECOVERY_OBJECT: v3 0x009A67D4 0x009A67D8 ?ClimateRiseAccum@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_rise_accum_
// RECOVERY_OBJECT: v3 0x009A67D8 0x009A67DC ?ClimateSeaLevelDelta@@3HA 0x00524210 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.climate_sea_level_delta_
// RECOVERY_OBJECT: v3 0x009A6670 0x009A66C9 ?GameTechAchieved@@3PAEA 0x004DFAD0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.game_tech_achieved_
// RECOVERY_OBJECT: v3 0x009A64BC ?TutWinFlags9A64BC@@3HA member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.tutwin_flags_9a64bc_
// RECOVERY_OBJECT: v3 0x009A64DC 0x009A64E0 ?VehSelectionFollowLimit@@3HA 0x005C08C0 member ?GameDataStateStorage@@3UGameDataStateWindow@@A fields_.vehicle_selection_follow_limit_
// game_data transfers exactly 0x398 bytes from 0x009A6490 (0x005A6C24..
// 0x005A6C9A), ending where the separately bounded Proposal table begins.
// RECOVERY_OBJECT: v2 0x009A6490 0x009A6828 ?GameDataStateStorage@@3UGameDataStateWindow@@A 0x005A6AC0
GameDataStateWindow GameDataStateStorage;
// The council proposal table, 0x009A6828 in the shipped image - MaxProposalNum
// entries of 0xc bytes. Was `RulesProposal *const Proposal =
// (RulesProposal *)0x009A6828` in council.h, naming terranx.exe data that is
// unmapped in a standalone build; see council.h. Zero at load there, as here:
// alpha.cpp's PROPOSALS text load fills it before anything reads it.
// RECOVERY_OBJECT: v2 0x009A6828 0x009A68AC ?Proposal@@3PAURulesProposal@@A 0x005873C0
RulesProposal Proposal[MaxProposalNum];  // 0x009A6828, 0xc stride
// MapLandmark at 0x00949894 is MapStorage.metadata_.landmarks_ (map.cpp).
// Bound: 128 x 0x1C records; reset loop walks +4 to `cmp eax, 0x9ab534` (0x005C55D4).
// RECOVERY_OBJECT: v2 0x009AA730 0x009AB530 ?Continents@@3PAUContinent@@A 0x005C55D4
Continent Continents[MaxContinentNum];                      // 0x009AA730
// Ability-model assembly reads the signed prerequisite at record +0x18;
// the original indexes these existing rules with a 0x1C-byte stride.
// Bound: 29 x 0x1C records; loops end at `cmp edi, 0x9ab864` (0x00485641).
// RECOVERY_OBJECT: v2 0x009AB538 0x009AB864 ?Ability@@3PAURulesAbility@@A 0x00485641
RulesAbility Ability[MaxAbilityNum];  // 0x009AB538
// ===== MANAGED GLOBALS - the veh.h tables, homed here =====
// Zero initialisation IS the image's initialisation: every base below reads
// ZERO at run time (`image_data.py`, all past the file-backed end of .data),
// and the alpha.txt / x.txt loaders fill them before use.
// The table ends where the next mapped object, ExitTurnLoop at 0x009B2068,
// begins: 0x009AB868 + 512 * 0x34 = 0x009B2068.
// RECOVERY_OBJECT: v2 0x009AB868 0x009B2068 ?VehPrototypes@@3PAUVehPrototype@@A 0x00493AF0
VehPrototype VehPrototypes[MaxVehProtoNum];  // 0x009AB868
// Original dword stop-loop flag, set by system-close and read by wait paths.
// RECOVERY_OBJECT: v2 0x009B2068 0x009B206C ?ExitTurnLoop@@3HA 0x004050C7
int ExitTurnLoop;       // 0x009B2068
// Earlier recovery interpreted 0x009A6504/0x009A6508 as independent
// EndgameFactionSelectorA/B scalars. They are FactionRankings[6]/[7],
// owned in alpha.cpp: rankings writes all eight slots, and setup_player
// reads these slots at 0x005B231F/0x005B2300. Keep the historical
// interpretation here; separate globals would sever producer/consumer state.
// RECOVERY_OBJECT: v2 0x009B206C 0x009B2070 ?EndgameExitReason@@3HA 0x004F2FFE
int EndgameExitReason;
// human_turn514263 writes this independent dword; draw_bases4671B5 reads it.
// Loader-zero storage ends before the separately owned GameSessionActive.
// RECOVERY_OBJECT: v2 0x009B2070 0x009B2074 ?ControlTurnPhase@@3HA 0x00514050
int ControlTurnPhase;
// Session lifecycle flag read by Console::load_proc and the new-game menu.
// RECOVERY_OBJECT: v2 0x009B2074 0x009B2078 ?GameSessionActive@@3HA 0x0058E360
int GameSessionActive;
// Original serializer transfers two adjacent, independent256-byte records.
// RECOVERY_OBJECT: v2 0x009B2078 0x009B2178 ?SaveNameBuffer@@3PADA 0x005A6AC0
char SaveNameBuffer[256];
// RECOVERY_OBJECT: v2 0x009B2178 0x009B2278 ?ScenarioSaveNameBuffer@@3PADA 0x005A6AC0
char ScenarioSaveNameBuffer[256];

// Unbound rules tables of the same run (address comments only), in image order.
// Each address is zero-initialised data in the shipped image (image_data.py:
// all sixteen sit in .data past the bytes the file carries, ZERO at run
// time), filled in by read_rules()/read_basic_rules()/read_tech() below.
// They were typed-pointer bindings into the image's address space until
// converted to the objects themselves; the folded absolute addressing of
// every use site is unchanged, the immediate now a relocation.
RulesResourceinfo ResourceInfo[MaxResourceInfoNum]; // 0x00945F50
LPSTR PlansShortName[MaxPlanNum];  // 0x00945FE0
RulesCombatMode OffenseModes[MaxOffenseModeNum];  // 0x00946178
RulesBonusName BonusName[MaxBonusNameNum];                  // 0x009461A8
RulesCombatMode DefenseModes[MaxDefenseModeNum];  // 0x00946A00
LPSTR Repute[MaxReputeNum];                                 // 0x00946A30
RulesEnergy Energy[MaxEnergyNum];                   // 0x0094A318
RulesMight Might[MaxMightNum];                              // 0x0094C558
LPSTR Mood[MaxMoodNum];                                     // 0x0094C9E4
LPSTR Triad[MaxTriadNum];  // 0x0094F1A8
LPSTR PlansFullName[MaxPlanNum];  // 0x00952360
LPSTR Difficulty[MaxDiffNum];      // 0x0096C85C
uint16_t FactionRankingHistory[MaxPlayerNum * MaxRankingHistoryTurns];  // 0x009A68AC

// ORIGINAL: 0x005BEC40 _$E21 0x005BEC40-0x005BEC56 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682660 selects initializer0x5bec40..0x5bec56, registering exit0x5bec60..0x5bec6a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x94c590 bound to?g_PALETTE1@@3VPalette@@A.", "Original startup instruction evidence: [[\"0x5bec40\", \"mov\", \"ecx, 0x94c590\"], [\"0x5bec45\", \"call\", \"0x5fe2a0\"], [\"0x5bec4a\", \"push\", \"0x5bec60\"], [\"0x5bec4f\", \"call\", \"0x645398\"], [\"0x5bec54\", \"pop\", \"ecx\"], [\"0x5bec55\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x5bec60\", \"mov\", \"ecx, 0x94c590\"], [\"0x5bec65\", \"jmp\", \"0x5fe2e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/interface_palette.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_PALETTE1@@3VPalette@@A\", \"??0Palette@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_PALETTE1@@3VPalette@@A\", \"??1Palette@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f53c; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_PALETTE1@@3VPalette@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E20
// ORIGINAL: 0x005BEC60 _$E19 0x005BEC60-0x005BEC6A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682660 selects initializer0x5bec40..0x5bec56, registering exit0x5bec60..0x5bec6a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x94c590 bound to?g_PALETTE1@@3VPalette@@A.", "Original startup instruction evidence: [[\"0x5bec40\", \"mov\", \"ecx, 0x94c590\"], [\"0x5bec45\", \"call\", \"0x5fe2a0\"], [\"0x5bec4a\", \"push\", \"0x5bec60\"], [\"0x5bec4f\", \"call\", \"0x645398\"], [\"0x5bec54\", \"pop\", \"ecx\"], [\"0x5bec55\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x5bec60\", \"mov\", \"ecx, 0x94c590\"], [\"0x5bec65\", \"jmp\", \"0x5fe2e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/interface_palette.cpp COFF selects _$E21 via .CRT$XCU, registering _$E19. Startup relocation targets: [\"?g_PALETTE1@@3VPalette@@A\", \"??0Palette@@QAE@XZ\", \"_$E19\", \"_atexit\"]; exit targets: [\"?g_PALETTE1@@3VPalette@@A\", \"??1Palette@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f53c; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_PALETTE1@@3VPalette@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E18
// RECOVERY_OBJECT: v2 0x0094C590 0x0094C9E4 ?g_PALETTE1@@3VPalette@@A 0x0058E360
Palette g_PALETTE1;

// ORIGINAL: 0x005BEC70 _$E26 0x005BEC70-0x005BEC86 FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682664 selects initializer0x5bec70..0x5bec86, registering exit0x5bec90..0x5bec9a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x9523a0 bound to?g_PALETTE2@@3VPalette@@A.", "Original startup instruction evidence: [[\"0x5bec70\", \"mov\", \"ecx, 0x9523a0\"], [\"0x5bec75\", \"call\", \"0x5fe2a0\"], [\"0x5bec7a\", \"push\", \"0x5bec90\"], [\"0x5bec7f\", \"call\", \"0x645398\"], [\"0x5bec84\", \"pop\", \"ecx\"], [\"0x5bec85\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x5bec90\", \"mov\", \"ecx, 0x9523a0\"], [\"0x5bec95\", \"jmp\", \"0x5fe2e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/interface_palette.cpp COFF selects _$E26 via .CRT$XCU, registering _$E24. Startup relocation targets: [\"?g_PALETTE2@@3VPalette@@A\", \"??0Palette@@QAE@XZ\", \"_$E24\", \"_atexit\"]; exit targets: [\"?g_PALETTE2@@3VPalette@@A\", \"??1Palette@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f540; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original startup operation to the genuine compiler-emitted lifetime of source-owned ?g_PALETTE2@@3VPalette@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E25
// ORIGINAL: 0x005BEC90 _$E24 0x005BEC90-0x005BEC9A FILE BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["Pinned CRT slot0x682664 selects initializer0x5bec70..0x5bec86, registering exit0x5bec90..0x5bec9a. Root independent complete decode confirms exclusive RET/JMP bounds; original/source-owned storage is0x9523a0 bound to?g_PALETTE2@@3VPalette@@A.", "Original startup instruction evidence: [[\"0x5bec70\", \"mov\", \"ecx, 0x9523a0\"], [\"0x5bec75\", \"call\", \"0x5fe2a0\"], [\"0x5bec7a\", \"push\", \"0x5bec90\"], [\"0x5bec7f\", \"call\", \"0x645398\"], [\"0x5bec84\", \"pop\", \"ecx\"], [\"0x5bec85\", \"ret\", \"\"]]. Original exit instruction evidence: [[\"0x5bec90\", \"mov\", \"ecx, 0x9523a0\"], [\"0x5bec95\", \"jmp\", \"0x5fe2e0\"]]. Preserve constructor arguments, array counts/strides, registration and reverse destruction without authored callback stand-ins.", "Fresh complete build-iveupb5l physical src/interface_palette.cpp COFF selects _$E26 via .CRT$XCU, registering _$E24. Startup relocation targets: [\"?g_PALETTE2@@3VPalette@@A\", \"??0Palette@@QAE@XZ\", \"_$E24\", \"_atexit\"]; exit targets: [\"?g_PALETTE2@@3VPalette@@A\", \"??1Palette@@QAE@XZ\"]. Actual linked helper occurs once at CRT slot0x63f540; selected same-owner order agrees with the pinned order.", "BaseWin/DesignWin definitions now precede their Wave globals as pinned. Existing DesignWin Wave mappings retain original names/spans and current helper overrides after fresh emission proof. Scalar constructor effects and array construction/cleanup belong to genuine source globals, classes and compiler-generated lifetime machinery.", "Current local labels are emission interpretations, not original debug names or recovered historical TU boundaries. Start bindings grant no allocation extent. Selected same-owner linked order is not full CRT/global-order, retained-selection, reference-target, dependency-provider, runtime or full-game acceptance. Guarded callbacks excluded; no claim grant or bug fix."], "reason": "Bind original exit operation to the genuine compiler-emitted lifetime of source-owned ?g_PALETTE2@@3VPalette@@A; preserve complete original identity and remaining restoration/structure obligations."}
// symbol    _$E23
// RECOVERY_OBJECT: v2 0x009523A0 0x009527F4 ?g_PALETTE2@@3VPalette@@A 0x00491400
Palette g_PALETTE2;  // 0x009523A0, the preferences gamma working palette

// ORIGINAL: 0x005BECA0 ?X_text_open@@YAHPADPBD@Z 0x005BECA0-0x005BED17 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["the current helper calls only text_open and compares the requested file to the process script-file pointer", "ClimatePopupTitle aliases &ScriptPopupFile; 0x00939284 is ConsoleGlobal.faction_id_; 0x00946f58 is Players[0].rule_flags with 0x59c stride", "src/game.h, src/console.h, src/faction.h and src/popup.h declare the corresponding real objects and flags; unrelated leaf bodies still use the two TU-local aliases, which this transfer leaves in place", "the current function checks only the local faction's 0x80 rule bit and ExpansionEnabled, tries PopupStartAltFile1, then falls back to the original requested file"], "reason": "Home the original section-opening policy beside Text::open/text_open, replacing transitional pointer aliases with existing source-owned script, local-faction, player-rule, expansion, and alternate-file objects without changing branch order."}
// size      119 bytes
// prototype int (__cdecl ?X_text_open@@YAHPADPBD@Z)(int8* srcFileID, int8* sectionID)
// callers   19   call targets   2
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005FD550 0x00645660
// notes     Seed provenance: the LEVER note records that the entry frame is
//           downstream of the strcmp intrinsic - inlining a string routine
//           forced esi/edi saves - and that the two globals are PTR_s_
//           slots holding a char*, one dereference more than the string.
// historical Return Value: text_open's result for the section
// historical Status: Measured BYTE_EXACT here in place (wave 7 record-then-retire); the
// LEVER: string-routines-are-calls "Original keeps a `push ebp; mov ebp, esp` frame this source form never reproduces (register-pressure shape, not yet found)" was the note here, and the shape has now been found: the scaffold inlined a string routine the image calls, the expansion needs esi and edi, and VC6 answered by changing what it saved on entry. The prologue was downstream of the intrinsic. The `g_00691b0c`/`g_00691b20` reading stands - they are PTR_s_ globals holding a `char*`, one dereference more than the address of the string itself.
int __cdecl X_text_open(char *src_file_id, const char *section_id) {
    if (strcmp(src_file_id, ScriptPopupFile) != 0) { return text_open(src_file_id, section_id); }
    const int faction_id = ConsoleGlobal.faction_id_;
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) != 0 && ExpansionEnabled != 0) {
        if (text_open(PopupStartAltFile1, section_id) == 0) { return 0; }
    }
    return text_open(src_file_id, section_id);
}


// RECOVERY_ASSOCIATED: v1 switch_targets 0x005BED20 0x005BEFE0 0x005BEFF0 0x005BED70
// RECOVERY_ASSOCIATED: v1 switch_selector 0x005BED20 0x005BEFF0 0x005BF004 0x005BED6A
// ORIGINAL: 0x005BED20 ?do_unity_crash@@YAHHHH@Z 0x005BED20-0x005BEFDF BYTE_EXACT
// RECOVERY_BINDING: v1 {"reason":"Recover Map bit2 event dispatch, spawn/morale and turn-based energy awards, and the live Base table radar scan with real UI/audio objects. The seed note calling this a unit scan is historical; the original stride134 and nativeBase count prove Bases. No byte/reference or runtime claim is granted.","evidence":["git:3f579310:src/unrecovered/005bed20.cpp","image:01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf","recovery/tasks/scenario-artifact-unity-owner-coverage.json","Original full span and native physical COFF emitted symbol ?do_unity_crash@@YAHHHH@Z; existing source owns all referenced storage and services."]}
// NOTE: code bytes match the image with relocations masked; recover measure
//       refuses it while ConsoleInfoWin/WorldClimateWorldWin lack bounded
//       object spans. The image keeps two "unitfound_sm.pcx" literals (unpooled).
// RECOVERY_LITERAL: v1 0x00691BF8 0x00691C09 $SG65350 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C0C 0x00691C19 $SG65351 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C1C 0x00691C2D $SG65354 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C30 0x00691C3B $SG65355 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C3C 0x00691C4C $SG65358 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C4C 0x00691C58 $SG65359 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C58 0x00691C60 $SG65366 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C60 0x00691C72 $SG65368 0x005BED20
// RECOVERY_LITERAL: v1 0x00691C74 0x00691C7F $SG65369 0x005BED20
int __cdecl do_unity_crash(int x, int y, int faction_id) {
    const uint32_t event = MapTiles[static_cast<int>(MapLongitude) * y + (x >> 1)].bit2;
    if ((event & 0x80004000u) != 0x4000u) {
        return 0;
    }

    const int event_kind = static_cast<int>(event) >> 24;
    switch (event_kind) {
    case 0:
    case 8: {
        int prototype;
        if (event_kind) {
            prototype = 0x0C;
            if (faction_id == ConsoleGlobal.faction_id_) {
                g_FX.play(0x16);
                NetMsgPrimary.pop("UNITYCHOPPER", -5000, 0, "unitfound_sm.pcx");
                g_FX.fade(0x16);
            }
        } else {
            prototype = 0x14;
            if (faction_id == ConsoleGlobal.faction_id_) {
                g_FX.play(0x16);
                NetMsgPrimary.pop("UNITYMINER", -5000, 0, "unitfound_sm.pcx");
                g_FX.fade(0x16);
            }
        }
        Vehs[veh_init(prototype, faction_id, x, y)].morale = 2;
        return 1;
    }
    case 10: {
        // The award expression is written twice; VC6 CSEs it into one value.
        PlayersData[faction_id].energy_reserves += (GameDataStateStorage.fields_.turn_current_num_ < 0x32) ? 150 : 300;
        parse_num(0, (GameDataStateStorage.fields_.turn_current_num_ < 0x32) ? 150 : 300);
        ConsoleInfoWin.on_redraw();
        if (faction_id == ConsoleGlobal.faction_id_) {
            g_FX.play(0x16);
            NetMsgPrimary.pop("UNITYENGINE", -5000, 0, "foundeng_sm.pcx");
            g_FX.fade(0x16);
        }
        return 1;
    }
    case 0x13: {
        for (int base_id = 0; base_id < GameDataStateStorage.fields_.base_current_count_; base_id++) {
            StringTemp[0] = 0;
            strcat(StringTemp, Players[Bases[base_id].faction_id_current].filename);
            CharUpperA(StringTemp);
            if (has_fac(1, base_id, 0)
                && (Players[Bases[base_id].faction_id_current].rule_flags & RFLAG_ALIEN) == 0
                && strcmp(StringTemp, "FUNGBOY") != 0) {
                spot_loc(Bases[base_id].x, Bases[base_id].y, faction_id);
            }
        }
        if (faction_id == ConsoleGlobal.faction_id_) {
            draw_map(1);
            WorldClimateWorldWin.field_239BC_ |= 4;
            WorldClimateWorldWin.GraphicWin::redraw();
            g_FX.play(0x16);
            NetMsgPrimary.pop("UNITYRADAR", -5000, 0, "foundradar_sm.pcx");
            g_FX.fade(0x16);
        }
        return 1;
    }
    }
    return 0;
}

/*
Purpose: Check whether there is a sensor available in the specified tile.
// ORIGINAL: 0x005BF010 ?is_sensor@@YAHHH@Z 0x005BF010-0x005BF12D BYTE_EXACT
// LEVER: a prior pass's comment read "removed unnecessary duplicate calculation of distX",
//        caching `x_dist(x, Bases[base_id].x)` into a local before the `!dist_x || dist_x
//        == 2` check. The image calls `x_dist` (0x00644F3A, abs()) TWICE - once per side of
//        the `||`, with no CSE across it - while `dist_y` (a plain `abs()`, not `x_dist`) IS
//        computed once and cached, exactly as this tree already had it. Restoring the
//        second `x_dist` call moved the compiled instruction count from 73 toward the
//        image's 102 (73 -> 87).
// TRIED: remaining divergence starts in the prologue - `mov ecx,[ebp+0xc]` (image) vs
//            this tree spilling y to ebx (`push ebx; mov ebx,[ebp+0xc]`) - the same
//            register-allocation plateau as the rest of this family.
// size      285 bytes
// prototype int (__cdecl ?is_sensor@@YAHHH@Z)(int xCoord, int yCoord)
// callers   8   call targets   3
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3B80 0x0050BA00 0x00644F3A
Return Value: 0 (no sensor), 1 (sensor array via terraforming), 2 (Geosynchronous Survey Pod)
Status: Complete
*/
int __cdecl is_sensor(int x, int y) {
    if (bit_at(x, y) & BIT_SENSOR_ARRAY) {
        return 1; // Sensor Array built in tile
    }
    int base_id = base_find(x, y);
    if (base_id != -1) {
        if (!x_dist(x, Bases[base_id].x) || x_dist(x, Bases[base_id].x) == 2) {
            int dist_y = y - Bases[base_id].y;
            if (!dist_y || (dist_y >= 0 ? dist_y : Bases[base_id].y - y) == 2) {
                if (has_fac(FAC_GEOSYNC_SURVEY_POD, base_id, 0)) {
                    return 2; // Geosynchronous Survey Pod
                }
            }
        }
    }
    return 0; // No sensor found
}


/*
Purpose: Check if faction controls the initial tile (code offset 0) of the Manifold Nexus.
// ORIGINAL: 0x005BF130 ?has_temple@@YAHH@Z 0x005BF130-0x005BF1EE BYTE_EXACT
// LEVER: needs /Oy- (the marker's own `frame` flag) to even form the ebp-frame
//        prologue the image has; that alone was 3/70 -> 51/70. Caching
//        `uint32_t bit2 = map_loc(x, y)->bit2;` once and testing
//        `bit2 & MASK` / `!(bit2 >> 24)` against the LOCAL (rather than
//        `bit2_at(x, y)` / `code_at(x, y)` as two separate inline calls) let
//        VC6 fold the `*sizeof(Map)` scale into the load's own SIB addressing
//        the way the image does, taking it to 65/70, 0.986 similar.
//        TRIED: caching a `Map *tile` pointer instead of the `bit2` value
//        (65->21/70 - the pointer materialises with its own `lea` first,
//        which the image never does); swapping the final visibility test's
//        `&` operand order (no change, still 65/70) - the one remaining
//        differing run (`map_loc(x, y)->visibility & (1 << faction_id)`,
//        register choice only: esi/edx/eax vs image's eax/ecx/edx for the
//        same three values) did not move under either.
// TRIED (2026-09-03): hoisting `int mask = 1 << faction_id` ahead of the
//            loop, dropping the `bit2` local in favour of two inline
//            accessor calls (51/70, worse), and swapping the `&` operands -
//            all still 65/70. The image keeps the tile index in eax across
//            the whose_territory call and puts the mask in edx; this tree
//            puts the index in esi and the mask in eax.
// size      190 bytes
// prototype int (__cdecl ?has_temple@@YAHH@Z)(int factionID)
// callers   1   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x004E3EF0
Return Value: Does faction control Nexus? true/false
Status: Complete
*/
BOOL __cdecl has_temple(int faction_id) {
    for (int y = 0; y < MapLatitudeBounds; y++) {
        for (int x = y & 1; x < MapLongitudeBounds; x += 2) {
            uint32_t bit2 = map_loc(x, y)->bit2;
            if ((bit2 & (BIT2_UNK_80000000 | BIT2_NEXUS)) == BIT2_NEXUS
                && !(bit2 >> 24)
                && faction_id == whose_territory(faction_id, x, y, NULL, false)
                && map_loc(x, y)->visibility & (1 << faction_id)) { // tile visible
                return true;
            }
        }
    }
    return false;
}

/*
Purpose: Check whether the prototype has a specific ability.
// ORIGINAL: 0x005BF1F0 ?has_abil@@YAHHH@Z 0x005BF1F0-0x005BF310 BYTE_EXACT
// TRIED: a negative-proto_id guard clause (with its log_say call) that call_diff flagged as an extra call the image does not make; now 13/109 instructions agree, 0.825 similar (best flags), up from a structurally-unrelated function body before
// size      288 bytes
// prototype int (__cdecl ?has_abil@@YAHHH@Z)(int protoID, int abilityID)
// callers   54   call targets   1
// kind      game
// flags     frame;sp_ready;purged_ok
// calls     0x005B9F20
Return Value: Does prototype have ability? true/false
Status: Complete
*/
BOOL __cdecl has_abil(int proto_id, int ability_id) {
    // BUG IN THE ORIGINAL: no guard against a negative proto_id here - it flows straight
    // into `proto_id / MaxVehProtoFactionNum` and an out-of-bounds VehPrototypes[] read.
    // A prior pass added a guard clause (with a log_say call) that is NOT in the image;
    // call_diff flagged the extra call, and the disassembly at 0x005BF1F0 confirms the
    // first instructions compute proto_id*sizeof(VehPrototype) directly, no branch first.
    int faction_id = proto_id / MaxVehProtoFactionNum;
    if (VehPrototypes[proto_id].ability_flags & ability_id) {
        return true;
    }
    if (!faction_id) {
        return false; // skip basic prototypes from #UNITS
    }
    if ((Players[faction_id].rule_flags & RFLAG_ALIEN) && ability_id == ABL_DEEP_RADAR) {
        return true; // Caretakers + Usurpers > "Deep Radar" ability for all units
    }
    for (int i = 0; i < Players[faction_id].faction_bonus_count; i++) {
        if (Players[faction_id].faction_bonus_id[i] == RULE_FREEABIL) {
            // Original 0x5BF298 rereads the bonus value after has_tech returns.
            if (has_tech(Ability[Players[faction_id].faction_bonus_val1[i]].preq_tech, faction_id) &&
                (ability_id & (1 << Players[faction_id].faction_bonus_val1[i]))) {
                // Pirates > "Marine Detachment" ability for combat sea units with Adaptive Doctrine
                return true;
            }
        }
    }
    if (VehPrototypes[proto_id].weapon_id == WPN_PROBE_TEAM && ability_id == ABL_ALGO_ENHANCEMENT
        && has_project(SP_NETHACK_TERMINUS, faction_id)) {
        return true; // All Probe Teams act as though they have the "Algorithmic Enhancement"
    }
    return false;
}

// The shared popup implementation owns these dialogs in popup.cpp. Keep the
// declarations local to this TU: the public popup header does not expose them.
int __cdecl pop_ask(char *text_file, char *text_label, int value,
                    char *text, int (__cdecl *callback)(), int flags);
int __cdecl pop_ask_number(char *caption, char *label, int value,
                           int (__cdecl *callback)());

// THE SAME BUFFER `popup.cpp` calls `PopupStartCaption` - one array at
// 0x009B8AA8 under two names. Defined there; used here.

/*
Purpose: Raise a popup with caller-supplied override text, using its string
         length and the default pair of enabled flags.
// size      60 bytes
// kind      game
Return Value: whatever the full form returns, or -1 for a null text pointer
Status: Complete
*/
// ORIGINAL: 0x005BF6F0 ?X_pops@@YAHPADPBDPADHPAUSprite@@P6AHXZ@Z 0x005BF6F0-0x005BF72C BYTE_EXACT
// symbol    ?X_pops@@YAHPADPBD0HPAVSprite@@P6AHXZ@Z
int __cdecl X_pops(char *caption, const char *label, char *text, int value,
                   Sprite *sprite, int (__cdecl *callback)()) {
    if (text == nullptr) {
        return -1;
    }
    const int length = static_cast<int>(strlen(text));
    return X_pops(caption, label, length, text, value, sprite, 1, 1,
                  callback);
}

/*
Purpose: Raise a popup with caller-supplied override text and flags.
// size      64 bytes
// kind      game
Return Value: whatever the full form returns, or -1 for a null text pointer
Status: Complete
*/
// ORIGINAL: 0x005BF730 ?X_pops@@YAHPADPBDPADHPAUSprite@@HHP6AHXZ@Z 0x005BF730-0x005BF770 BYTE_EXACT
// symbol    ?X_pops@@YAHPADPBD0HPAVSprite@@HHP6AHXZ@Z
int __cdecl X_pops(char *caption, const char *label, char *text, int value,
                   Sprite *sprite, int flag1, int flag2,
                   int (__cdecl *callback)()) {
    if (text == nullptr) {
        return -1;
    }
    return X_pops(caption, label, static_cast<int>(strlen(text)), text,
                  value, sprite, flag1, flag2, callback);
}

/*
Purpose: Raise a popup with the defaults the short form implies - the shared
         caption buffer, no title, no override text, and both flags set.
// ORIGINAL: 0x005BF7D0 ?X_pops@@YAHPBDHPAUSprite@@P6AHXZ@Z 0x005BF7D0-0x005BF7FA BYTE_EXACT
// symbol    ?X_pops@@YAHPBDHPAVSprite@@P6AHXZ@Z
// size      42 bytes
// prototype 
// callers   27   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl X_pops(const char *label, int value, Sprite *sprite,
                         int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, -1, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: The three-argument popup form: like the short form but with the value
         defaulted to zero and no override text.
// ORIGINAL: 0x005BF5D0 ?X_pops@@YAHPBDPAUSprite@@P6AHXZ@Z 0x005BF5D0-0x005BF5F8 BYTE_EXACT
// symbol    ?x_pops_minimal@@YAHPBDPAVSprite@@P6AHXZ@Z
// size      40 bytes
// prototype 
// callers   6   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_minimal(const char *label, Sprite *sprite,
                           int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, -1, nullptr, 0,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the caption-less popup with its own caption and label supplied
         by the caller, rather than the shared buffer - value, override text
         and flags all take their defaults (-1, none, 0).
// ORIGINAL: 0x005BF330 ?X_pop@@YAHPADPBDP6AHXZ@Z 0x005BF330-0x005BF34F BYTE_EXACT
// symbol    ?X_pop@@YAHPADPBDP6AHXZ@Z
// size      31 bytes
// prototype 
// callers   17   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl X_pop(char *caption, const char *label,
                                int (__cdecl *callback)()) {
    return X_pop(caption, label, -1, nullptr, 0, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer, with
         both the value and the flags supplied by the caller. No override
         text.
// ORIGINAL: 0x005BF390 ?X_pop@@YAHPBDHHP6AHXZ@Z 0x005BF390-0x005BF3B4 BYTE_EXACT
// symbol    ?x_pop_value_flags@@YAHPBDHHP6AHXZ@Z
// size      36 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_value_flags(const char *label, int value, int flags,
                              int (__cdecl *callback)()) {
    return X_pop(PopupStartCaption, label, value, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption, value
         and flags. No override text.
// ORIGINAL: 0x005BF3C0 ?X_pop@@YAHPADPBDHHP6AHXZ@Z 0x005BF3C0-0x005BF3E3 BYTE_EXACT
// symbol    ?x_pop_caption_value_flags@@YAHPADPBDHHP6AHXZ@Z
// size      35 bytes
// prototype 
// callers   2   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_caption_value_flags(char *caption, const char *label,
                                     int value, int flags,
                                     int (__cdecl *callback)()) {
    return X_pop(caption, label, value, nullptr, flags, callback);
}

/*
Purpose: The short form of the caption-less popup - shared caption buffer,
         value defaulted to -1, no override text, only the flags exposed.
// ORIGINAL: 0x005BF3F0 ?X_pop@@YAHPBDHP6AHXZ@Z 0x005BF3F0-0x005BF412 BYTE_EXACT
// symbol    ?x_pop_short@@YAHPBDHP6AHXZ@Z
// size      34 bytes
// prototype 
// callers   17   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_short(const char *label, int flags,
                        int (__cdecl *callback)()) {
    return X_pop(PopupStartCaption, label, -1, nullptr, flags,
                            callback);
}

/*
Purpose: Raise the caption-less popup with a caller-supplied caption and
         flags; value defaults to -1 and there is no override text.
// ORIGINAL: 0x005BF420 ?X_pop@@YAHPADPBDHP6AHXZ@Z 0x005BF420-0x005BF441 BYTE_EXACT
// symbol    ?x_pop_caption_flags@@YAHPADPBDHP6AHXZ@Z
// size      33 bytes
// prototype 
// callers   4   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_caption_flags(char *caption, const char *label, int flags,
                                int (__cdecl *callback)()) {
    return X_pop(caption, label, -1, nullptr, flags, callback);
}

/*
Purpose: Raise the caption-less popup against the shared caption buffer with
         everything else - value, override text and flags - supplied by the
         caller.
// ORIGINAL: 0x005BF450 ?X_pop@@YAHPBDHPADHP6AHXZ@Z 0x005BF450-0x005BF476 BYTE_EXACT
// symbol    ?x_pop_default_caption@@YAHPBDHPADHP6AHXZ@Z
// size      38 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF480
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pop_default_caption(const char *label, int value, char *text,
                                  int flags, int (__cdecl *callback)()) {
    return X_pop(PopupStartCaption, label, value, text, flags,
                            callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1, no override text and value forced to 0; the sprite
         and both flags are caller-supplied. Unlike x_pops_short/minimal, the
         flags are not fixed to 1/1 here.
// ORIGINAL: 0x005BF600 ?X_pops@@YAHPBDPAUSprite@@HHP6AHXZ@Z 0x005BF600-0x005BF62C BYTE_EXACT
// symbol    ?x_pops_flags@@YAHPBDPAVSprite@@HHP6AHXZ@Z
// size      44 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_flags(const char *label, Sprite *sprite, int flag1,
                         int flag2, int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, -1, nullptr, 0,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: The minimal popup form with a caller-supplied caption instead of the
         shared buffer - title -1, no override text, value 0, both flags
         fixed at 1.
// ORIGINAL: 0x005BF630 ?X_pops@@YAHPADPBDPAUSprite@@P6AHXZ@Z 0x005BF630-0x005BF657 BYTE_EXACT
// symbol    ?x_pops_caption_minimal@@YAHPADPBDPAVSprite@@P6AHXZ@Z
// size      39 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_minimal(char *caption, const char *label,
                                   Sprite *sprite,
                                   int (__cdecl *callback)()) {
    return X_pops(caption, label, -1, nullptr, 0, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_flags but with a caller-supplied caption in place of the
         shared buffer - title -1, no override text, value forced to 0, both
         flags caller-supplied.
// ORIGINAL: 0x005BF660 ?X_pops@@YAHPADPBDPAUSprite@@HHP6AHXZ@Z 0x005BF660-0x005BF68B BYTE_EXACT
// symbol    ?x_pops_caption_flags@@YAHPADPBDPAVSprite@@HHP6AHXZ@Z
// size      43 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_flags(char *caption, const char *label,
                                 Sprite *sprite, int flag1, int flag2,
                                 int (__cdecl *callback)()) {
    return X_pops(caption, label, -1, nullptr, 0, sprite, flag1,
                             flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with both
         title and value exposed (unlike the short form, which only exposes
         value); no override text, both flags fixed at 1.
// ORIGINAL: 0x005BF690 ?X_pops@@YAHPBDHHPAUSprite@@P6AHXZ@Z 0x005BF690-0x005BF6BC BYTE_EXACT
// symbol    ?x_pops_title@@YAHPBDHHPAVSprite@@P6AHXZ@Z
// size      44 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_title(const char *label, int title, int value,
                         Sprite *sprite, int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, title, nullptr, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Like x_pops_title but with both flags caller-supplied instead of
         fixed at 1; shared caption buffer, no override text.
// ORIGINAL: 0x005BF6C0 ?X_pops@@YAHPBDHHPAUSprite@@HHP6AHXZ@Z 0x005BF6C0-0x005BF6F0 BYTE_EXACT
// symbol    ?x_pops_title_flags@@YAHPBDHHPAVSprite@@HHP6AHXZ@Z
// size      48 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_title_flags(const char *label, int title, int value,
                               Sprite *sprite, int flag1, int flag2,
                               int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, title, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_title but with a caller-supplied caption instead of the
         shared buffer; no override text, both flags fixed at 1.
// ORIGINAL: 0x005BF770 ?X_pops@@YAHPADPBDHHPAUSprite@@P6AHXZ@Z 0x005BF770-0x005BF79B BYTE_EXACT
// symbol    ?x_pops_caption_title@@YAHPADPBDHHPAVSprite@@P6AHXZ@Z
// size      43 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_title(char *caption, const char *label, int title,
                                 int value, Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return X_pops(caption, label, title, nullptr, value, sprite, 1,
                             1, callback);
}

/*
Purpose: Raise the full popup with everything but the override text
         caller-supplied - caption, title, value, sprite and both flags.
// ORIGINAL: 0x005BF7A0 ?X_pops@@YAHPADPBDHHPAUSprite@@HHP6AHXZ@Z 0x005BF7A0-0x005BF7CF BYTE_EXACT
// symbol    ?x_pops_no_text@@YAHPADPBDHHPAVSprite@@HHP6AHXZ@Z
// size      47 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_no_text(char *caption, const char *label, int title,
                           int value, Sprite *sprite, int flag1, int flag2,
                           int (__cdecl *callback)()) {
    return X_pops(caption, label, title, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title
         defaulted to -1 and no override text; value and both flags are
         caller-supplied.
// ORIGINAL: 0x005BF800 ?X_pops@@YAHPBDHPAUSprite@@HHP6AHXZ@Z 0x005BF800-0x005BF82E BYTE_EXACT
// symbol    ?x_pops_value_flags@@YAHPBDHPAVSprite@@HHP6AHXZ@Z
// size      46 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_value_flags(const char *label, int value, Sprite *sprite,
                               int flag1, int flag2,
                               int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, -1, nullptr, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Like x_pops_value_flags but with a caller-supplied caption in place
         of the shared buffer and both flags fixed at 1; title defaults to
         -1, no override text.
// ORIGINAL: 0x005BF830 ?X_pops@@YAHPADPBDHPAUSprite@@P6AHXZ@Z 0x005BF830-0x005BF859 BYTE_EXACT
// symbol    ?x_pops_caption_value@@YAHPADPBDHPAVSprite@@P6AHXZ@Z
// size      41 bytes
// prototype 
// callers   1   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_value(char *caption, const char *label, int value,
                                 Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return X_pops(caption, label, -1, nullptr, value, sprite, 1, 1,
                             callback);
}

/*
Purpose: Like x_pops_caption_value but with both flags caller-supplied
         instead of fixed at 1; title defaults to -1, no override text.
// ORIGINAL: 0x005BF860 ?X_pops@@YAHPADPBDHPAUSprite@@HHP6AHXZ@Z 0x005BF860-0x005BF88D BYTE_EXACT
// symbol    ?x_pops_caption_value_flags@@YAHPADPBDHPAVSprite@@HHP6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_caption_value_flags(char *caption, const char *label,
                                      int value, Sprite *sprite, int flag1,
                                      int flag2,
                                      int (__cdecl *callback)()) {
    return X_pops(caption, label, -1, nullptr, value, sprite,
                             flag1, flag2, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with title,
         override text and value all caller-supplied; both flags fixed at 1.
// ORIGINAL: 0x005BF890 ?X_pops@@YAHPBDHPADHPAUSprite@@P6AHXZ@Z 0x005BF890-0x005BF8BE BYTE_EXACT
// symbol    ?x_pops_override_text@@YAHPBDHPADHPAVSprite@@P6AHXZ@Z
// size      46 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_override_text(const char *label, int title, char *text,
                                 int value, Sprite *sprite,
                                 int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, title, text, value,
                             sprite, 1, 1, callback);
}

/*
Purpose: Raise the full popup against the shared caption buffer with every
         other argument - title, override text, value, sprite and both flags
         - caller-supplied. Only the caption defaults.
// ORIGINAL: 0x005BF8C0 ?X_pops@@YAHPBDHPADHPAUSprite@@HHP6AHXZ@Z 0x005BF8C0-0x005BF8F2 BYTE_EXACT
// symbol    ?x_pops_default_caption@@YAHPBDHPADHPAVSprite@@HHP6AHXZ@Z
// size      50 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_default_caption(const char *label, int title, char *text,
                                   int value, Sprite *sprite, int flag1,
                                   int flag2, int (__cdecl *callback)()) {
    return X_pops(PopupStartCaption, label, title, text, value,
                             sprite, flag1, flag2, callback);
}

/*
Purpose: Raise the full popup with caption, title, override text, value and
         sprite all caller-supplied; only the two flags default, fixed at 1.
// ORIGINAL: 0x005BF900 ?X_pops@@YAHPADPBDHPADHPAUSprite@@P6AHXZ@Z 0x005BF900-0x005BF92D BYTE_EXACT
// symbol    ?x_pops_no_flags@@YAHPADPBDH0HPAVSprite@@P6AHXZ@Z
// size      45 bytes
// prototype 
// callers   0   call targets   1
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x005BF930
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl x_pops_no_flags(char *caption, const char *label, int title,
                            char *text, int value, Sprite *sprite,
                            int (__cdecl *callback)()) {
    return X_pops(caption, label, title, text, value, sprite, 1, 1,
                             callback);
}

// Eight fixed strings X_pop reaches for the mod-key check below: two
// filename/mode pairs it fopen/fclose-probes, two override captions/labels
// it compares against, and two override-caption pointers it substitutes.
// REAL LITERALS AND SLOTS since 2026-08-29 - the raw .rdata bindings
// (0x691B7C, 0x691C80, 0x691C8C, 0x691C90, 0x691CA4, 0x691CA8) and the
// three pointer slots (0x691B0C, 0x691B20, 0x691B24, initialized in the
// image to "SCRIPT", "alienIscript", "alienuscript") all pointed into
// terranx.exe's read-only data, unmapped in the standalone build - the
// very first X_pop call faulted reading the second compare string.
// Each wrapper below has five distinct literal starts in the original. Its
// local arrays preserve that storage identity; original spelling and
// translation-unit boundaries remain unknown.
// The three SLOT-HELD ones stay behind inline accessors reading through
// a real slot variable, which is the one-instruction double indirection
// the image's `mov eax, [slot]` has.
inline LPCSTR XPopModCheckString1() {
    return ScriptPopupFile;
}
inline LPCSTR XPopModCheckCaption1() {
    return PopupStartAltFile1;
}
inline LPCSTR XPopModCheckCaption2() {
    return PopupStartAltFile2;
}

/*
Purpose: The full builder every xpops wrapper funnels into. When the label
         matches one of two fixed override strings, probe for a mod-key file
         (opening and immediately closing it) gated on the human's own or
         the popup dialog's faction having its spying flag set, and on
         expansions being enabled; on success substitute a fixed caption for
         the popup. Either way, fall through to the shared pop builder.
// ORIGINAL: 0x005BF480 ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z 0x005BF480-0x005BF5C2 BYTE_EXACT
// NOTE: needs the ebp frame (optimize "y" off) and pop_full args written inline.
// symbol    ?X_pop@@YAHPADPBDH0HP6AHXZ@Z
// size      322 bytes
// prototype int (__cdecl ?X_pop@@YAHPADPBDHPADHP6AHXZ@Z)(int8*, int8*, int length, int8*, int, int (__cdecl *)())
// callers   9   call targets   4
// kind      game
// flags     frame;hidden;sp_ready;purged_ok
// calls     0x006272C0 0x00645598 0x00645646 0x00645660
Return Value: whatever the shared pop builder returns
Status: Complete
*/
int __cdecl X_pop(char *caption, const char *label, int a3, char *a4, int a5,
                  int (__cdecl *callback)()) {

    char *const label2 = const_cast<char *>(label);
    // THE ALIEN FLAG, not a "spying" flag: 0x00946F58 is &Players[0].
    // rule_flags (the old binding's own address proves it - Players sits at
    // 0x00946A50 and rule_flags walks to +0x508), and bit 0x80 is
    // RFLAG_ALIEN. That is why the probe files are alienIscript.txt and
    // alienuscript.txt: only an expansion alien faction redirects these
    // two captions.
    if (strcmp(caption, XPopModCheckString1()) == 0 ||
// RECOVERY_LITERAL: v1 0x00691C80 0x00691C8B $SG65617 0x005BF480
        strcmp(caption, "SCRIPT.txt") == 0) {
        if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
            if (ExpansionEnabled != 0) {
// RECOVERY_LITERAL: v1 0x00691C90 0x00691CA1 $SG65622 0x005BF480
// RECOVERY_LITERAL: v1 0x00691C8C 0x00691C8F $SG65621 0x005BF480
                FILE *const fp = fopen("alienIscript.txt", "rb");
                if (fp != nullptr) {
                    fclose(fp);
                    int const result = pop_full(const_cast<char *>(XPopModCheckCaption1()), label2, a3, a4, a5, callback);
                    if (result != -1) {
                        return result;
                    }
                }
            }
        } else if (ExpansionEnabled != 0 && 0 < PopupDialogFactionID &&
                  PopupDialogFactionID < 8 &&
                  (Players[PopupDialogFactionID].rule_flags &
                   RFLAG_ALIEN) != 0) {
// RECOVERY_LITERAL: v1 0x00691CA8 0x00691CB9 $SG65631 0x005BF480
// RECOVERY_LITERAL: v1 0x00691CA4 0x00691CA7 $SG65630 0x005BF480
            FILE *const fp = fopen("alienuscript.txt", "rb");
            if (fp != nullptr) {
                fclose(fp);
                int const result = pop_full(const_cast<char *>(XPopModCheckCaption2()), label2, a3, a4, a5, callback);
                if (result != -1) {
                    return result;
                }
            }
        }
        return pop_full(caption, label2, a3, a4, a5, callback);
    }
    return pop_full(caption, label2, a3, a4, a5, callback);
}

/*
Purpose: The two-argument popup form: caption from the shared buffer, no
         value, no sprite.
// ORIGINAL: 0x005BF310 ?X_pop@@YAHPBDP6AHXZ@Z 0x005BF310-0x005BF330 BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/005bf310.cpp, where the claim proved the artifact and the shipped program held nothing at this address. The artifact reached the caption as `(int8 *)g_009b8aa8`; popup.cpp already models that buffer as PopupStartCaption, and xpops.cpp already includes popup.h for exactly this - the sibling forms above use the same name.
// size      32 bytes
// kind      game
Return Value: whatever the full form returns
Status: Complete
*/
int __cdecl X_pop(const char *label, int (__cdecl *callback)()) {
    return X_pop(PopupStartCaption, label, -1, nullptr, 0, callback);
}

// Text-bearing X_pop overload: reject a missing text buffer, otherwise pass
// its actual length to the common six-argument implementation.
// ORIGINAL: 0x005BF350 ?X_pop@@YAHPADPBDPADHP6AHXZ@Z 0x005BF350-0x005BF384 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/recovered/005bf350.cpp", "source:src/xpops.h"], "reason": "Recover the caption, label, override-text and value overload as a null-checked length-calculating wrapper over the existing typed X_pop core."}
// symbol    ?X_pop@@YAHPADPBD0HP6AHXZ@Z
int __cdecl X_pop(char *caption, const char *label, char *text, int value,
                  int (__cdecl *callback)()) {
    if (text == nullptr) return -1;
    return X_pop(caption, label, static_cast<int>(strlen(text)), text,
                 value, callback);
}

// X_pop_ask with shared caption, caller-provided initial value and text.
// ORIGINAL: 0x005BFAB0 ?X_pop_ask@@YAHPBDHPADP6AHXZH@Z 0x005BFAB0-0x005BFAD6 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/recovered/005bfab0.cpp", "source:src/xpops.h"], "reason": "Recover the shared-caption X_pop_ask overload as a typed delegation to its captioned body."}
int __cdecl X_pop_ask(const char *label, int value, char *text,
                      int (__cdecl *callback)(), int flags) {
    return X_pop_ask(PopupStartCaption, label, value, text, callback, flags);
}

// Captioned X_pop_ask with no initial text buffer.
// ORIGINAL: 0x005BFAE0 ?X_pop_ask@@YAHPADPBDHP6AHXZH@Z 0x005BFAE0-0x005BFB03 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/recovered/005bfae0.cpp", "source:src/xpops.h"], "reason": "Recover the captioned X_pop_ask overload that defaults its optional text buffer to null."}
int __cdecl X_pop_ask(char *caption, const char *label, int value,
                      int (__cdecl *callback)(), int flags) {
    return X_pop_ask(caption, label, value, nullptr, callback, flags);
}

// Captioned X_pop_ask overload that derives its initial length from text.
// ORIGINAL: 0x005BFB40 ?X_pop_ask@@YAHPADPBDPADP6AHXZH@Z 0x005BFB40-0x005BFB74 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/recovered/005bfb40.cpp", "source:src/xpops.h"], "reason": "Recover the text-bearing X_pop_ask overload with null guard and computed text length."}
// symbol    ?X_pop_ask@@YAHPADPBD0P6AHXZH@Z
int __cdecl X_pop_ask(char *caption, const char *label, char *text,
                      int (__cdecl *callback)(), int flags) {
    if (text == nullptr) return -1;
    return X_pop_ask(caption, label, static_cast<int>(strlen(text)),
                     text, callback, flags);
}

// Core text-dialog routing. Match the mod-sensitive caption tokens, probe
// the corresponding optional script file, then fall back to the caller's
// caption if the alternate dialog cannot be opened or accepted.
// ORIGINAL: 0x005BFBC0 ?X_pop_ask@@YAHPADPBDHPADP6AHXZH@Z 0x005BFBC0-0x005BFD02 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/unrecovered/005bfbc0.cpp", "source:src/xpops.h", "source:src/popup.h", "source:src/faction.h"], "reason": "Recover the mod-aware shared X_pop_ask core through typed faction, popup-file, and popup-dialog owners."}
// symbol    ?X_pop_ask@@YAHPADPBDH0P6AHXZH@Z
int __cdecl X_pop_ask(char *caption, const char *label, int value,
                      char *text, int (__cdecl *callback)(), int flags) {

    char *const section = const_cast<char *>(label);
    if (strcmp(caption, XPopModCheckString1()) != 0 &&
// RECOVERY_LITERAL: v1 0x00691CF8 0x00691D03 $SG65683 0x005BFBC0
        strcmp(caption, "SCRIPT.txt") != 0) {
        return pop_ask(caption, section, value, text, callback, flags);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        if (ExpansionEnabled != 0) {
// RECOVERY_LITERAL: v1 0x00691D08 0x00691D19 $SG65688 0x005BFBC0
            FILE *const file = fopen("alienIscript.txt",
// RECOVERY_LITERAL: v1 0x00691D04 0x00691D07 $SG65687 0x005BFBC0
                                          "rb");
            if (file != nullptr) {
                fclose(file);
                const int result = pop_ask(PopupStartAltFile1, section, value,
                                           text, callback, flags);
                if (result != -1) return result;
            }
        }
    } else if (ExpansionEnabled != 0 && PopupDialogFactionID > 0 &&
               PopupDialogFactionID < 8 &&
               (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) != 0) {
// RECOVERY_LITERAL: v1 0x00691D20 0x00691D31 $SG65696 0x005BFBC0
        FILE *const file = fopen("alienuscript.txt",
// RECOVERY_LITERAL: v1 0x00691D1C 0x00691D1F $SG65695 0x005BFBC0
                                      "rb");
        if (file != nullptr) {
            fclose(file);
            const int result = pop_ask(PopupStartAltFile2, section, value,
                                       text, callback, flags);
            if (result != -1) return result;
        }
    }

    return pop_ask(caption, section, value, text, callback, flags);
}

/*
Purpose: The three-argument numeric-entry popup form: caption from the shared
         buffer, value and poll callback as given.
// ORIGINAL: 0x005BFD10 ?X_pop_ask_number@@YAHPBDHP6AHXZ@Z 0x005BFD10-0x005BFD2E BYTE_EXACT
// LEVER: PROMOTED out of src/recovered/005bfd10.cpp on the same terms as
//        0x005BF310 above - the claim proved the artifact and nothing in the
//        shipped program owned the address. DipEdit::on_button_clicked is the
//        compiled caller that needed it (the integrity prompt).
// size      30 bytes
// kind      game
// calls     0x005BFD70
Return Value: whatever the four-argument form returns
Status: Complete
*/
int __cdecl X_pop_ask_number(const char *label, int value,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(PopupStartCaption, label, value, callback);
}

/*
Purpose: The caption-taking numeric-entry form with the value defaulted.
         Preserve the supplied caption and use the original zero default.
// size      27 bytes
// kind      game
Return Value: whatever the four-argument form returns
Status: Complete
*/
// ORIGINAL: 0x005BFD30 ?X_pop_ask_number@@YAHPADPBDP6AHXZ@Z 0x005BFD30-0x005BFD4B BYTE_EXACT
// symbol    ?X_pop_ask_number@@YAHPADPBDP6AHXZ@Z
int __cdecl X_pop_ask_number(char *caption, const char *label,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(caption, label, 0, callback);
}

// Numeric-entry convenience form: shared caption and initial value zero.
// ORIGINAL: 0x005BFD50 ?X_pop_ask_number@@YAHPBDP6AHXZ@Z 0x005BFD50-0x005BFD6C BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:8f04e84382e6d278311ffe49ba2305afaea25ed7:src/recovered/005bfd50.cpp", "source:src/xpops.h", "source:src/popup.h"], "reason": "Recover the shared-caption numeric X_pop_ask_number convenience overload with its original zero default."}
int __cdecl X_pop_ask_number(const char *label,
                             int (__cdecl *callback)()) {
    return X_pop_ask_number(PopupStartCaption, label, 0, callback);
}

// Full numeric-entry routing mirrors X_pop_ask's alien-script fallback,
// with the alternate file/caption pair selected by the same faction gates.
// ORIGINAL: 0x005BFD70 ?X_pop_ask_number@@YAHPADPBDHP6AHXZ@Z 0x005BFD70-0x005BFE8E BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["git:276d612614085e5d57743cf8cc92903d9468e346:src/unrecovered/005bfd70.cpp", "src/xpops.h", "src/popup.h", "src/faction.h"], "reason": "Recover numeric X_pop_ask_number mod-caption and alien-script routing using typed popup and faction state."}
// TRIED: `#pragma function(strcmp)` to stop VC6 inlining the two `strcmp` calls into a byte-compare loop (without it, similarity was 0.61 with 38 extra mnemonics from the inlined loop; with it, 0.76). Calling `fopen`/`fclose` through a cast function pointer INLINE (not via an intermediate local) to avoid materialising their address in a register first, same lever as 0x004C5E50's import calls. Landed at 0.76: the remaining gap is a `push edi` register-save the original keeps for a callee-saved register this body's simpler live-range set does not need.
int __cdecl X_pop_ask_number(char *caption, const char *label, int value,
                             int (__cdecl *callback)()) {

    char *const section = const_cast<char *>(label);
    if (strcmp(caption, XPopModCheckString1()) != 0 &&
// RECOVERY_LITERAL: v1 0x00691D34 0x00691D3F $SG65723 0x005BFD70
        strcmp(caption, "SCRIPT.txt") != 0) {
        return pop_ask_number(caption, section, value, callback);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        if (ExpansionEnabled != 0) {
// RECOVERY_LITERAL: v1 0x00691D44 0x00691D55 $SG65728 0x005BFD70
            FILE *const file = fopen("alienIscript.txt",
// RECOVERY_LITERAL: v1 0x00691D40 0x00691D43 $SG65727 0x005BFD70
                                          "rb");
            if (file != nullptr) {
                fclose(file);
                const int result = pop_ask_number(PopupStartAltFile1, section,
                                                  value, callback);
                if (result != -1) return result;
            }
        }
    } else if (ExpansionEnabled != 0 && PopupDialogFactionID > 0 &&
               PopupDialogFactionID < 8 &&
               (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) != 0) {
// RECOVERY_LITERAL: v1 0x00691D5C 0x00691D6D $SG65736 0x005BFD70
        FILE *const file = fopen("alienuscript.txt",
// RECOVERY_LITERAL: v1 0x00691D58 0x00691D5B $SG65735 0x005BFD70
                                      "rb");
        if (file != nullptr) {
            fclose(file);
            const int result = pop_ask_number(PopupStartAltFile2, section,
                                              value, callback);
            if (result != -1) return result;
        }
    }

    return pop_ask_number(caption, section, value, callback);
}

// ORIGINAL: 0x005BFB10 ?X_pop_ask@@YAHPBDHP6AHXZH@Z 0x005BFB10-0x005BFB34 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x005BFB10-0x005BFB34", "source:src/xpops.cpp:typed owner", "source:src/xpops.h:X_pop_ask overload family", "source:src/popup.cpp:PopupStartCaption and pop_ask_number owner", "seed:retained original source identity 0x005BFB10"], "reason": "The short X_pop_ask overload supplies the shared popup caption and forwards the original callback and flags to the existing text-dialog core."}
int __cdecl X_pop_ask(const char *label, int value,
                      int (__cdecl *callback)(), int flags) {
    return X_pop_ask(PopupStartCaption, label, value,
                     static_cast<char *>(0), callback, flags);
}

// The original text wrapper calls strlen out of line. Use the real CRT API,
// with VC6's explicit function selection rather than a scaffold alias.

// ORIGINAL: 0x005BFB80 ?X_pop_ask@@YAHPBDPADP6AHXZH@Z 0x005BFB80-0x005BFBC0 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x005BFB80-0x005BFBC0", "source:src/xpops.cpp:typed owner", "source:src/xpops.h:X_pop_ask overload family", "source:src/popup.cpp:existing text-dialog core", "seed:retained original source identity 0x005BFB80"], "reason": "The text-bearing X_pop_ask overload preserves its null-text return and derives length with the existing C string API before forwarding."}
int __cdecl X_pop_ask(const char *label, char *text,
                      int (__cdecl *callback)(), int flags) {
    if (text == 0) {
        return -1;
    }
    return X_pop_ask(PopupStartCaption, label, static_cast<int>(strlen(text)), text,
                     callback, flags);
}

// ORIGINAL: 0x005BF930 ?X_pops@@YAHPADPBDHPADHPAUSprite@@HHP6AHXZ@Z 0x005BF930-0x005BFAA2 BYTE_EXACT
// RECOVERY_BINDING: v1 {"evidence": ["original:terranx_original.exe:0x005BF930-0x005BFAA2", "source:src/xpops.cpp:typed owner", "source:src/xpops.cpp:XPopModCheck literals/accessors and related routed overloads", "source:src/xpops.h:X_pops family", "source:src/popup.h:shared pops core", "seed:retained original source identity 0x005BF930"], "reason": "The full X_pops router belongs in xpops.cpp with its existing typed faction, caption/file literals, expansion flag, and shared popup core; it replaces only the pending executable forwarder."}
// symbol    ?X_pops@@YAHPADPBDH0HPAVSprite@@HHP6AHXZ@Z
// TRIED: precomputing the magic-string deref into a named local before the strcmp calls (#2 mismatch, 'mov' vs 'push'); same codegen.
int __cdecl X_pops(char *caption, const char *label, int text_length,
                   char *text, int value, Sprite *sprite, int flag_a,
                   int flag_b, int (__cdecl *callback)()) {

    char *const mutable_label = const_cast<char *>(label);
    if (strcmp(caption, XPopModCheckString1()) != 0 &&
// RECOVERY_LITERAL: v1 0x00691CBC 0x00691CC7 $SG65770 0x005BF930
        strcmp(caption, "SCRIPT.txt") != 0) {
        return pops(caption, mutable_label, text_length, text, value,
                    sprite, flag_a, flag_b, callback);
    }

    if ((Players[ConsoleGlobal.faction_id_].rule_flags & RFLAG_ALIEN) != 0) {
        if (ExpansionEnabled != 0) {
// RECOVERY_LITERAL: v1 0x00691CCC 0x00691CDD $SG65775 0x005BF930
            FILE *const file = fopen("alienIscript.txt",
// RECOVERY_LITERAL: v1 0x00691CC8 0x00691CCB $SG65774 0x005BF930
                                          "rb");
            if (file != 0) {
                fclose(file);
                const int result = pops(
                    const_cast<char *>(XPopModCheckCaption1()), mutable_label,
                    text_length, text, value, sprite, flag_a, flag_b, callback);
                if (result != -1) {
                    return result;
                }
            }
        }
    } else if (ExpansionEnabled != 0 && PopupDialogFactionID > 0 &&
               PopupDialogFactionID < 8 &&
               (Players[PopupDialogFactionID].rule_flags & RFLAG_ALIEN) != 0) {
// RECOVERY_LITERAL: v1 0x00691CE4 0x00691CF5 $SG65784 0x005BF930
        FILE *const file = fopen("alienuscript.txt",
// RECOVERY_LITERAL: v1 0x00691CE0 0x00691CE3 $SG65783 0x005BF930
                                      "rb");
        if (file != 0) {
            fclose(file);
            const int result = pops(
                const_cast<char *>(XPopModCheckCaption2()), mutable_label,
                text_length, text, value, sprite, flag_a, flag_b, callback);
            if (result != -1) {
                return result;
            }
        }
    }
    return pops(caption, mutable_label, text_length, text, value,
                sprite, flag_a, flag_b, callback);
}

// The load-game path uses the primary short X_pops body (original
// 0x005BF7D0..0x005BF7FA); the password warning uses captioned X_pop
// (original 0x005BF330..0x005BF34F). Their former shared bindings retained
// evidence anchor 0x005AAAB0. Both duplicate definitions are consolidated
// into the primary bodies above; no additional callable identity.
