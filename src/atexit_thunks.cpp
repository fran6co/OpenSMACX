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
#include "original_seam.h"
#include "atexit_thunks.h"
#include "ambience.h"
#include "battlewin.h"
#include "buffer.h"
#include "buttongroup.h"
#include "caviar.h"
#include "font.h"
#include "fx.h"
#include "sprite.h"
#include "texture.h"
#include "wave.h"

func_wave_destructor WaveOriginalDestructor =
    original_method<func_wave_destructor>(0x004C67C0);


// Per-element teardowns the array walks dispatch to. The Wave one binds
// the same address as WaveOriginalDestructor deliberately: it is the same
// original body, held separately so the array seam rebinds alone.
func_thiscall_teardown BufferElementTeardown =
    original_method<func_thiscall_teardown>(0x005D7410);
func_thiscall_teardown CaviarDataElementTeardown =
    original_method<func_thiscall_teardown>(0x00456100);
func_thiscall_teardown FactionArtElementTeardown =
    original_method<func_thiscall_teardown>(0x00456010);
func_thiscall_teardown FontElementTeardown =
    original_method<func_thiscall_teardown>(0x00618EE0);
func_thiscall_teardown SpriteElementTeardown =
    original_method<func_thiscall_teardown>(0x00406850);
func_thiscall_teardown TextIndexElementTeardown =
    original_method<func_thiscall_teardown>(0x005FDF60);
func_thiscall_teardown TextureElementTeardown =
    original_method<func_thiscall_teardown>(0x00619660);
func_thiscall_teardown WaveElementTeardown =
    original_method<func_thiscall_teardown>(0x004C67C0);

// The opaque-storage teardowns, each defaulting to the
// original destructor the thunk tail jumps to; the
// hybrid redirects it to a recovered body at run time.
func_thiscall_teardown InfoWinDtorTarget =
    original_method<func_thiscall_teardown>(0x004594A0);
func_thiscall_teardown MessageWinDtorTarget =
    original_method<func_thiscall_teardown>(0x00472020);
func_thiscall_teardown ReportIfDtorTarget =
    original_method<func_thiscall_teardown>(0x004ACDA0);
func_thiscall_teardown StatusWinDtorTarget =
    original_method<func_thiscall_teardown>(0x004BA120);
func_thiscall_teardown Wave_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C4E60);
func_thiscall_teardown Midi_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C5780);
func_thiscall_teardown Wave_In_DeviceDtorTarget =
    original_method<func_thiscall_teardown>(0x004C5980);
func_thiscall_teardown NetDaemonDtorTarget =
    original_method<func_thiscall_teardown>(0x00538D10);
func_thiscall_teardown PaletteDtorTarget =
    original_method<func_thiscall_teardown>(0x005FE2E0);
func_thiscall_teardown TimeDtorTarget =
    original_method<func_thiscall_teardown>(0x00616200);
func_thiscall_teardown StringsDtorTarget =
    original_method<func_thiscall_teardown>(0x006169C0);
func_thiscall_teardown CaviarCloseTarget =
    original_method<func_thiscall_teardown>(0x00617020);

Wave *g_ALPHAMENU_WAVE = (Wave *)0x006A7090;
Sprite *g_UNUSED_SPRITE_VAR02 = (Sprite *)0x006A7130;
Sprite *g_UNUSED_SPRITE_VAR11 = (Sprite *)0x006A72E0;
Sprite *g_UNUSED_SPRITE_VAR06 = (Sprite *)0x006A71F0;
Sprite *g_UNUSED_SPRITE_VAR09 = (Sprite *)0x006A7280;
Sprite *g_UNUSED_SPRITE_VAR21 = (Sprite *)0x006A74C0;
Sprite *g_UNUSED_SPRITE_VAR05 = (Sprite *)0x006A71C0;
Sprite *g_UNUSED_SPRITE_VAR08 = (Sprite *)0x006A7250;
Sprite *g_UNUSED_SPRITE_VAR04 = (Sprite *)0x006A7190;
Sprite *g_UNUSED_SPRITE_VAR01 = (Sprite *)0x006A7100;
Sprite *g_UNUSED_SPRITE_VAR18 = (Sprite *)0x006A7430;
Sprite *g_UNUSED_SPRITE_VAR03 = (Sprite *)0x006A7160;
Sprite *g_UNUSED_SPRITE_VAR20 = (Sprite *)0x006A7490;
Sprite *g_UNUSED_SPRITE_VAR16 = (Sprite *)0x006A73D0;
Sprite *g_UNUSED_SPRITE_VAR14 = (Sprite *)0x006A7370;
Sprite *g_UNUSED_SPRITE_VAR22 = (Sprite *)0x006A74F0;
Sprite *g_UNUSED_SPRITE_VAR10 = (Sprite *)0x006A72B0;
Sprite *g_UNUSED_SPRITE_VAR15 = (Sprite *)0x006A73A0;
Sprite *g_UNUSED_SPRITE_VAR13 = (Sprite *)0x006A7340;
Sprite *g_UNUSED_SPRITE_VAR17 = (Sprite *)0x006A7400;
Sprite *g_UNUSED_SPRITE_VAR19 = (Sprite *)0x006A7460;
Sprite *g_UNUSED_SPRITE_VAR12 = (Sprite *)0x006A7310;
Sprite *g_UNUSED_SPRITE_VAR07 = (Sprite *)0x006A7220;
Wave *g_BASEWIN_WAVE = (Wave *)0x006EEE68;
BattleWin *g_BattleWin = (BattleWin *)0x006EEED8;
Wave *g_CREDITS_WAVE = (Wave *)0x00703E30;
Wave *g_DESIGNWIN_WAVE = (Wave *)0x0071F240;
Wave *g_CPU_WAVES = (Wave *)0x0074C5F0;
Wave *g_MENU_UP_WAVE = (Wave *)0x00749C88;
Wave *g_MENU_DOWN_WAVE = (Wave *)0x0074D960;
Wave *g_SCOOT_WAVE = (Wave *)0x00749C18;
Wave *g_OK_WAVE = (Wave *)0x0074D8F0;
Wave *g_PASSOVER_WAVE = (Wave *)0x0074D9D0;
FX *g_FX = (FX *)0x00749CF8;
Ambience *g_AMBIENCE = (Ambience *)0x0074DA40;
Buffer *g_PCX_PARSE_TEMP_BUFFER1 = (Buffer *)0x00798668;
FactionArt *FactionArtGlobal = (FactionArt *)0x0078E978;
Sprite *g_IFACE_CLOSE_X_SPRITES = (Sprite *)0x007794D8;
Sprite *g_IFACE_BOX_SPRITES1 = (Sprite *)0x007AC290;
Sprite *g_IFACE_BOX_SPRITES2 = (Sprite *)0x0076DC98;
Sprite *g_IFACE_BOX_SPRITES3 = (Sprite *)0x0078AE58;
Sprite *g_IFACE_BOX_SPRITES4 = (Sprite *)0x0078CC60;
Sprite *g_IFACE_BOX_SPRITES5 = (Sprite *)0x00779CB0;
Sprite *g_IFACE_BOX_SPRITES6 = (Sprite *)0x007A6978;
Sprite *g_IFACE_BOX_SPRITES7 = (Sprite *)0x0078E0A0;
Sprite *g_IFACE_BOX_SPRITES8 = (Sprite *)0x0075B950;
Buffer *g_IFACE_BOX_SPRITE_BUFFERS = (Buffer *)0x0075C218;
Sprite *g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE = (Sprite *)0x007921E8;
Sprite *g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE = (Sprite *)0x00779478;
Sprite *g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE = (Sprite *)0x007AC1C0;
Sprite *g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE = (Sprite *)0x007AB400;
Sprite *g_IFACE_STD_POPUPS_TOP_MID_SPRITE = (Sprite *)0x007793F0;
Sprite *g_IFACE_STD_POPUPS_BOT_MID_SPRITE = (Sprite *)0x007871D0;
Sprite *g_IFACE_STD_POPUPS_MID_LEFT_SPRITE = (Sprite *)0x007ACFB0;
Sprite *g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE = (Sprite *)0x0075B058;
Buffer *g_IFACE_STD_POPUPS_MIDDLE_BUFFER = (Buffer *)0x0077AA70;
CaviarData *g_UNUSED_CAVIARDATA_VAR1 = (CaviarData *)0x00791D68;
CaviarData *g_SSF_CAVIARDATA = (CaviarData *)0x007ACCA0;
CaviarData *g_SDP_CAVIARDATA = (CaviarData *)0x0079A6A0;
CaviarData *g_SAS_CAVIARDATA = (CaviarData *)0x00787E48;
CaviarData *g_SCD_CAVIARDATA = (CaviarData *)0x00799228;
CaviarData *g_SCJ_CAVIARDATA = (CaviarData *)0x007ACC90;
CaviarData *g_SAGS_CAVIARDATA = (CaviarData *)0x0077A578;
CaviarData *g_SFT_CAVIARDATA = (CaviarData *)0x0079A6B0;
CaviarData *g_VHR_CAVIARDATA1 = (CaviarData *)0x007765B0;
CaviarData *g_SHT_CAVIARDATA = (CaviarData *)0x0078A5E0;
CaviarData *g_SRB_CAVIARDATA = (CaviarData *)0x0078A300;
CaviarData *g_ASAS_CAVIARDATA = (CaviarData *)0x007AC030;
CaviarData *g_RESLASER_CAVIARDATA = (CaviarData *)0x007ACCC0;
CaviarData *g_RESBOLT_CAVIARDATA = (CaviarData *)0x007A7900;
CaviarData *g_FUNLOAD_CAVIARDATA = (CaviarData *)0x007ACC10;
CaviarData *g_TECLOAD_CAVIARDATA = (CaviarData *)0x00788028;
CaviarData *g_SP_DISSWAVE_CAVIARDATA = (CaviarData *)0x00799E38;
CaviarData *g_SP_MARINED_CAVIARDATA = (CaviarData *)0x007792A0;
CaviarData *g_SP_NANOO_CAVIARDATA = (CaviarData *)0x0075AEF0;
CaviarData *g_SP_SOPORIFIC_CAVIARDATA = (CaviarData *)0x007AD130;
CaviarData *g_AA01_CAVIARDATA = (CaviarData *)0x007AD040;
CaviarData *g_AA_ROVER_CAVIARDATA = (CaviarData *)0x0075AFC0;
CaviarData *g_AA_WING_CAVIARDATA = (CaviarData *)0x00779288;
CaviarData *g_ACP_CAVIARDATA = (CaviarData *)0x0078A610;
CaviarData *g_AX_CAVIARDATA = (CaviarData *)0x0079A6F0;
CaviarData *g_AA_CAVIARDATA = (CaviarData *)0x00789A50;
CaviarData *g_ACOLPOD_CAVIARDATA = (CaviarData *)0x0078A600;
CaviarData *g_AT_CAVIARDATA = (CaviarData *)0x00787E60;
CaviarData *g_VTA_CAVIARDATA = (CaviarData *)0x007880C0;
CaviarData *g_ATP_CAVIARDATA = (CaviarData *)0x007ABFC0;
CaviarData *g_SSFA_CAVIARDATA = (CaviarData *)0x0078A510;
CaviarData *g_SFTA_CAVIARDATA = (CaviarData *)0x0079A640;
CaviarData *g_VW00_CAVIARDATA = (CaviarData *)0x007871C0;
CaviarData *g_VIPTAWL_CAVIARDATA = (CaviarData *)0x0079A700;
CaviarData *g_VIPTASGN_CAVIARDATA = (CaviarData *)0x007AB3F0;
CaviarData *g_VIPTAPSI_CAVIARDATA = (CaviarData *)0x007795A0;
CaviarData *g_VIPTR_CAVIARDATA = (CaviarData *)0x00799628;
CaviarData *g_VIPTA_CAVIARDATA = (CaviarData *)0x007791D0;
CaviarData *g_PTMOD_CAVIARDATA = (CaviarData *)0x0078A500;
CaviarData *g_VB_CAVIARDATA = (CaviarData *)0x00779230;
CaviarData *g_VBP_CAVIARDATA = (CaviarData *)0x0078E968;
CaviarData *g_VGMC_CAVIARDATA = (CaviarData *)0x0075AED0;
CaviarData *g_VGMCP_CAVIARDATA = (CaviarData *)0x0076EBE0;
CaviarData *g_VLIGHTS_CAVIARDATA = (CaviarData *)0x0075B848;
CaviarData *g_VPT_CAVIARDATA = (CaviarData *)0x007592D0;
CaviarData *g_A_CAVIARDATA = (CaviarData *)0x0075AF00;
CaviarData *g_APWALL_CAVIARDATA = (CaviarData *)0x0078A490;
CaviarData *g_ASGEN_CAVIARDATA = (CaviarData *)0x007AD120;
CaviarData *g_APSID_CAVIARDATA = (CaviarData *)0x007ACCB0;
CaviarData *g_VA01_CAVIARDATA = (CaviarData *)0x007AD0E0;
CaviarData *g_VR_CAVIARDATA = (CaviarData *)0x0076E560;
CaviarData *g_VHR_CAVIARDATA2 = (CaviarData *)0x00779560;
CaviarData *g_VRC_CAVIARDATA = (CaviarData *)0x0075ADB8;
CaviarData *g_VI_CAVIARDATA = (CaviarData *)0x0078B740;
CaviarData *g_VGMT_CAVIARDATA = (CaviarData *)0x0077A5B8;
CaviarData *g_VGMTP_CAVIARDATA = (CaviarData *)0x0078B720;
CaviarData *g_UNUSED_CAVIARDATA_VAR2 = (CaviarData *)0x007ACF90;
CaviarData *g_VWNTU_CAVIARDATA = (CaviarData *)0x007796A0;
CaviarData *g_VT_CAVIARDATA = (CaviarData *)0x0075AD78;
CaviarData *g_DROP_CAVIARDATA = (CaviarData *)0x007ACD60;
CaviarData *g_DROPLET_CAVIARDATA = (CaviarData *)0x0076EBD0;
CaviarData *g_VCL_CAVIARDATA = (CaviarData *)0x007AD170;
CaviarData *g_UNUSED_CAVIARDATA_VAR3 = (CaviarData *)0x007795F0;
CaviarData *g_VCLT00_CAVIARDATA = (CaviarData *)0x00789B58;
CaviarData *g_VHT_VBP_CAVIARDATA = (CaviarData *)0x00779608;
CaviarData *g_VHTP_CAVIARDATA = (CaviarData *)0x0078A640;
CaviarData *g_VHTA0_CAVIARDATA = (CaviarData *)0x00779648;
CaviarData *g_VHTTP_CAVIARDATA = (CaviarData *)0x00759310;
CaviarData *g_VHTTPA0_CAVIARDATA = (CaviarData *)0x0078B750;
CaviarData *g_VSP_CAVIARDATA = (CaviarData *)0x007A6820;
CaviarData *g_VSPA0_CAVIARDATA = (CaviarData *)0x00779270;
CaviarData *g_VSPTF_CAVIARDATA = (CaviarData *)0x007A7290;
CaviarData *g_VSPTB_CAVIARDATA = (CaviarData *)0x007A7280;
CaviarData *g_VFL_CAVIARDATA = (CaviarData *)0x007ACC50;
CaviarData *g_VGS_CAVIARDATA = (CaviarData *)0x007A7240;
CaviarData *g_VGSP_CAVIARDATA = (CaviarData *)0x00798BF0;
CaviarData *g_VJTP_CAVIARDATA = (CaviarData *)0x007795B0;
CaviarData *g_VJT0_CAVIARDATA = (CaviarData *)0x0078A2B8;
CaviarData *g_VCU_CAVIARDATA = (CaviarData *)0x0075AEE0;
CaviarData *g_VCUP_CAVIARDATA = (CaviarData *)0x0075B440;
CaviarData *g_VCUA0_CAVIARDATA = (CaviarData *)0x0078CAB8;
CaviarData *g_VCUW_CAVIARDATA = (CaviarData *)0x0075B088;
CaviarData *g_VCT_CAVIARDATA = (CaviarData *)0x0079A680;
CaviarData *g_VCTP_CAVIARDATA = (CaviarData *)0x007ACFA0;
CaviarData *g_VCTB_CAVIARDATA = (CaviarData *)0x0076E890;
CaviarData *g_VCT0_CAVIARDATA = (CaviarData *)0x0079A628;
CaviarData *g_VWNTT_CAVIARDATA = (CaviarData *)0x0078B730;
CaviarData *g_VWNST_CAVIARDATA = (CaviarData *)0x0078DE20;
CaviarData *g_VWNAA_CAVIARDATA = (CaviarData *)0x007AC1A0;
CaviarData *g_VW_CAVIARDATA = (CaviarData *)0x00787100;
CaviarData *g_VM_CAVIARDATA = (CaviarData *)0x00791D78;
CaviarData *g_VM13_CAVIARDATA = (CaviarData *)0x0078A5F0;
CaviarData *g_VPBR0_CAVIARDATA = (CaviarData *)0x0076E9B0;
CaviarData *g_UNUSED_CAVIARDATA_VAR4 = (CaviarData *)0x007791E8;
CaviarData *g_NW_CAVIARDATA = (CaviarData *)0x0079A690;
CaviarData *g_NI_CAVIARDATA = (CaviarData *)0x007791C0;
CaviarData *g_NLC_CAVIARDATA = (CaviarData *)0x00779660;
CaviarData *g_UNUSED_CAVIARDATA_VAR5 = (CaviarData *)0x00787E08;
Texture *g_RADIUS1_TEXTURE = (Texture *)0x00787FB8;
Texture *g_RADIUS2_TEXTURE = (Texture *)0x0075B858;
Texture *g_ROCKY_TEXTURES = (Texture *)0x0076E9E0;
Texture *g_OCEAN_TEXTURES = (Texture *)0x007A7820;
Texture *g_FLAT_ARID_LAND_TEXTURE = (Texture *)0x00799E48;
Texture *g_MOIST_LAND_TEXTURES = (Texture *)0x00799EB8;
Texture *g_RAINY_LAND_TEXTURES = (Texture *)0x00799738;
Texture *g_JUNGLE_LAND_TEXTURES = (Texture *)0x00789C28;
Texture *g_DUNE_LAND_TEXTURE = (Texture *)0x007AC220;
Texture *g_SUNNY_MESA_TEXTURES = (Texture *)0x007797F8;
Texture *g_RAINFALL_SINGLE_TILE_TEXTURES = (Texture *)0x0076E8A0;
Texture *g_ROAD_TEXTURES = (Texture *)0x00792218;
Texture *g_MAGTUBE_TEXTURES = (Texture *)0x00798E08;
Texture *g_RIVER_TEXTURES = (Texture *)0x007A7AA0;
Texture *g_MOUNT_PLANET_TEXTURES = (Texture *)0x0078A340;
Texture *g_GARLAND_CRATER_TEXTURES = (Texture *)0x00788100;
Texture *g_FUNGUS_TEXTURES = (Texture *)0x00776A80;
Texture *g_FARM_TEXTURES = (Texture *)0x00799238;
Texture *g_FOREST_TEXTURES = (Texture *)0x0078A758;
Texture *g_RAINFALL_SINGLE_TILE_TEXTURE = (Texture *)0x0078DD80;
Sprite *g_TER1_WHITE_ORG_YEL_TILE_SPRITES = (Sprite *)0x00791C58;
Sprite *g_TER1_BOTTOM_LEFT_TILE_SPRITES = (Sprite *)0x007A99A0;
Sprite *g_TER1_UNUSED_SPRITES2 = (Sprite *)0x00779420;
Sprite *g_TER1_MINE_SPRITE = (Sprite *)0x00776A50;
Sprite *g_TER1_SOLAR_COLLECTOR_SPRITE = (Sprite *)0x007991F8;
Sprite *g_TER1_TIDAL_HARNESS_SPRITE = (Sprite *)0x0078A5B0;
Sprite *g_TER1_MINING_PLATFORM_SPRITE = (Sprite *)0x00789B68;
Sprite *g_TER1_TUT_BLANK_SPRITE = (Sprite *)0x00798C40;
Sprite *g_TER1_KELP_FARM_SPRITE = (Sprite *)0x007A6860;
Sprite *g_TER1_CONDENSER_SPRITE = (Sprite *)0x00779390;
Sprite *g_TER1_ECHELON_MIRROR_SPRITE = (Sprite *)0x007ABF60;
Sprite *g_TER1_BOREHOLE_SPRITE = (Sprite *)0x0078DD20;
Sprite *g_TER1_BOREHOLE_CLUSTER_SPRITE = (Sprite *)0x007ACB88;
Sprite *g_TER1_MANIFOLD_NEXUS_SPRITES = (Sprite *)0x007AC098;
Sprite *g_TER1WRECK_UNITY_WRECKAGE_SPRITES = (Sprite *)0x00776758;
Sprite *g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES = (Sprite *)0x00799658;
Sprite *g_FOSSIL_FIELD_RIDGE_SPRITES = (Sprite *)0x007ACD70;
Sprite *g_TER1_UNUSED_SPRITES1 = (Sprite *)0x007792B0;
Sprite *g_TER1_FARM_SPRITES = (Sprite *)0x00791FC8;
Sprite *g_TER1_SOIL_ENRICHER_SPRITES = (Sprite *)0x0079A710;
Sprite *g_TER1_SEA_LAND_RESOURCE_SPRITES = (Sprite *)0x0075B230;
Sprite *g_TER1_LANDMARK_RESOURCE_SPRITES = (Sprite *)0x0078A650;
Sprite *g_GLOW_SPRITES = (Sprite *)0x00779770;
Sprite *g_TER1_UNITY_POD_SPRITES = (Sprite *)0x0077AFF8;
Sprite *g_TER1_MONOLITH_SPRITE = (Sprite *)0x0075B098;
Sprite *g_TER1_BUNKER_SPRITE = (Sprite *)0x007AD010;
Sprite *g_TER1_AIRBASE_SPRITE = (Sprite *)0x007ACC60;
Sprite *g_TER1_SENSOR_ARRAY_SPRITE = (Sprite *)0x007A6830;
Sprite *g_RAINFALL_DOUBLE_TILE_SPRITES = (Sprite *)0x007ACBB8;
Sprite *g_VEH_SPRITES = (Sprite *)0x007777A0;
Sprite *g_FLAGS_VEH_SPRITES = (Sprite *)0x0078B778;
Sprite *g_ICONS_GENERAL_SPRITES = (Sprite *)0x0075B450;
Sprite *g_RESOURCE_ICON_SPRITES = (Sprite *)0x007A72A0;
Sprite *g_CITIZEN_LG_CURSOR_SPRITES = (Sprite *)0x0078CAD0;
Sprite *g_SPECIALIST_LG_CURSOR_SPRITES = (Sprite *)0x007765C0;
Sprite *g_CITIZEN_SM_CURSOR_SPRITES = (Sprite *)0x00776420;
Sprite *g_SPECIALIST_SM_CURSOR_SPRITES = (Sprite *)0x00779B78;
Sprite *g_AL_CITIZEN_LG_CURSOR_SPRITES = (Sprite *)0x007AD1B0;
Sprite *g_AL_SPECIALIST_LG_CURSOR_SPRITES = (Sprite *)0x00798CD0;
Sprite *g_AL_CITIZEN_SM_CURSOR_SPRITES = (Sprite *)0x0075B180;
Sprite *g_AL_SPECIALIST_SM_CURSOR_SPRITES = (Sprite *)0x0075B710;
Sprite *g_RED_ALIEN_HEAD_ICON_SPRITE = (Sprite *)0x00791F98;
Sprite *g_SILVER_MENU_ICON_SPRITES = (Sprite *)0x0075AE20;
Sprite *g_SILVER_CHECKBOX_ICON_SPRITES = (Sprite *)0x0075AF68;
Sprite *g_RED_MALE_HEAD_ICON_SPRITE = (Sprite *)0x0075AD88;
Sprite *g_NULL_RESOURCE_ICON_SPRITE = (Sprite *)0x0078A520;
Sprite *g_PEACE_SIGN_SPRITES = (Sprite *)0x0075AF10;
Sprite *g_ICON_TILE_SQUARE_SPRITE = (Sprite *)0x00776728;
Sprite *g_XI_BOOM_VEH_SPRITES = (Sprite *)0x007A9B30;
Sprite *g_XF_BOOM_VEH_SPRITES = (Sprite *)0x007AB460;
Sprite *g_BATTLE_MIND_WORM_SPRITE = (Sprite *)0x00779200;
Sprite *g_BATTLE_ISLE_DEEP_SPRITE = (Sprite *)0x0076DC30;
Sprite *g_BATTLE_LOCUSTS_CHIRON_SPRITE = (Sprite *)0x0078A4A0;
Sprite *g_BATTLE_FUNGAL_TOWER_SPRITE = (Sprite *)0x00787E70;
Sprite *g_BATTLE_SPORE_LAUNCHER_SPRITE = (Sprite *)0x007AC000;
Sprite *g_BATTLE_SEALURK_SPRITE = (Sprite *)0x00779570;
Sprite *g_TECH_ICON_SPRITES = (Sprite *)0x00759E28;
Sprite *g_FACILITY_ICON_SPRITES = (Sprite *)0x00787200;
Sprite *g_SECRET_PROJECT_ICON_SPRITES = (Sprite *)0x00759320;
Sprite *g_IFACE_MP_COMBO_ARROW_SPRITES = (Sprite *)0x00788038;
Sprite *g_SCROLL_BAR_ARROW_ICON_SPRITES = (Sprite *)0x00791D88;
Sprite *g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES = (Sprite *)0x0078DE30;
Sprite *g_SCROLL_BAR_FILLER_ICON_SPRITES = (Sprite *)0x0079A5E8;
Sprite *g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE = (Sprite *)0x007796B0;
Sprite *g_IFACE_LOCK_SPRITES = (Sprite *)0x007AC040;
Sprite *g_UNUSED_SPRITES_VAR01 = (Sprite *)0x007A7940;
Sprite *g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE = (Sprite *)0x0078DD50;
Sprite *g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE = (Sprite *)0x00789BC8;
Sprite *g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE = (Sprite *)0x00776580;
Sprite *g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE = (Sprite *)0x007ABFD0;
Sprite *g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE = (Sprite *)0x00787F88;
Sprite *g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE = (Sprite *)0x0078A2D0;
Sprite *g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE = (Sprite *)0x0078E070;
Sprite *g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE = (Sprite *)0x0078A550;
Sprite *g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE = (Sprite *)0x00789BF8;
Sprite *g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE = (Sprite *)0x0075B150;
Sprite *g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE = (Sprite *)0x007AD180;
Sprite *g_UNUSED_SPRITE_VAR23 = (Sprite *)0x007880D0;
Sprite *g_UNUSED_SPRITE_VAR24 = (Sprite *)0x0079A5B8;
Sprite *g_UNUSED_SPRITE_VAR25 = (Sprite *)0x007797C8;
Sprite *g_UNUSED_SPRITE_VAR26 = (Sprite *)0x00792638;
Sprite *g_UNUSED_SPRITE_VAR27 = (Sprite *)0x0079A650;
Sprite *g_UNUSED_SPRITE_VAR28 = (Sprite *)0x00799708;
Sprite *g_UNUSED_SPRITE_VAR29 = (Sprite *)0x007ACFE0;
Sprite *g_UNUSED_SPRITE_VAR30 = (Sprite *)0x007A7910;
Sprite *g_UNUSED_SPRITE_VAR31 = (Sprite *)0x00779240;
Sprite *g_UNUSED_SPRITE_VAR32 = (Sprite *)0x007ACD00;
Sprite *g_UNUSED_SPRITE_VAR33 = (Sprite *)0x00776A20;
Sprite *g_UNUSED_SPRITE_VAR34 = (Sprite *)0x007769F0;
Sprite *g_UNUSED_SPRITE_VAR35 = (Sprite *)0x007796E0;
Sprite *g_UNUSED_SPRITE_VAR36 = (Sprite *)0x007AC1F0;
Sprite *g_UNUSED_SPRITE_VAR37 = (Sprite *)0x00789A60;
Sprite *g_UNUSED_SPRITE_VAR38 = (Sprite *)0x0078CC30;
Sprite *g_UNUSED_SPRITE_VAR39 = (Sprite *)0x00787F58;
Sprite *g_UNUSED_SPRITE_VAR40 = (Sprite *)0x00798C00;
Sprite *g_UNUSED_SPRITE_VAR41 = (Sprite *)0x0078A580;
Sprite *g_UNUSED_SPRITE_VAR42 = (Sprite *)0x007AD0F0;
Sprite *g_UNUSED_SPRITE_VAR43 = (Sprite *)0x007AD080;
Sprite *g_UNUSED_SPRITE_VAR44 = (Sprite *)0x00792608;
Sprite *g_UNUSED_SPRITE_VAR45 = (Sprite *)0x00787EA0;
Sprite *g_UNUSED_SPRITE_VAR46 = (Sprite *)0x007ACCD0;
Sprite *g_IFACE_TECH_TREE_ARROW_SPRITES = (Sprite *)0x0075AFD0;
Sprite *g_UNUSED_SPRITE_VAR47 = (Sprite *)0x00779740;
Sprite *g_UNUSED_SPRITE_VAR48 = (Sprite *)0x0078A4D0;
Sprite *g_UNUSED_SPRITE_VAR49 = (Sprite *)0x007AD140;
Sprite *g_UNUSED_SPRITE_VAR50 = (Sprite *)0x007793C0;
Sprite *g_UNUSED_SPRITE_VAR51 = (Sprite *)0x00789B98;
Sprite *g_UNUSED_SPRITE_VAR52 = (Sprite *)0x007766F8;
Sprite *g_UNUSED_SPRITE_VAR53 = (Sprite *)0x00798C70;
Sprite *g_UNUSED_SPRITE_VAR54 = (Sprite *)0x0076EBA0;
Sprite *g_UNUSED_SPRITE_VAR55 = (Sprite *)0x007592E0;
Sprite *g_UNUSED_SPRITE_VAR56 = (Sprite *)0x007795C0;
Sprite *g_UNUSED_SPRITE_VAR57 = (Sprite *)0x00792130;
Sprite *g_UNUSED_SPRITE_VAR58 = (Sprite *)0x0075ADF0;
Sprite *g_UNUSED_SPRITE_VAR59 = (Sprite *)0x0076E980;
Sprite *g_UNUSED_SPRITE_VAR60 = (Sprite *)0x0078A310;
Sprite *g_UNUSED_SPRITE_VAR61 = (Sprite *)0x0076DC60;
Sprite *g_UNUSED_SPRITE_VAR62 = (Sprite *)0x00789B18;
Sprite *g_UNUSED_SPRITE_VAR63 = (Sprite *)0x007A68C0;
Sprite *g_UNUSED_SPRITE_VAR64 = (Sprite *)0x0077A588;
Sprite *g_UNUSED_SPRITE_VAR65 = (Sprite *)0x00779710;
Sprite *g_UNUSED_SPRITE_VAR66 = (Sprite *)0x0079A6C0;
Sprite *g_UNUSED_SPRITE_VAR67 = (Sprite *)0x0078DDF0;
Sprite *g_UNUSED_SPRITE_VAR68 = (Sprite *)0x007A7250;
Sprite *g_UNUSED_SPRITE_VAR69 = (Sprite *)0x007AD0B0;
Sprite *g_UNUSED_SPRITE_VAR70 = (Sprite *)0x007AD050;
Sprite *g_UNUSED_SPRITE_VAR71 = (Sprite *)0x007A6890;
Sprite *g_UNUSED_SPRITE_VAR72 = (Sprite *)0x007763F0;
Sprite *g_UNUSED_SPRITE_VAR73 = (Sprite *)0x007ACD30;
Sprite *g_UNUSED_SPRITE_VAR74 = (Sprite *)0x007ABF90;
Sprite *g_UNUSED_SPRITES_VAR02 = (Sprite *)0x007A68F0;
Sprite *g_UNUSED_SPRITES_VAR03 = (Sprite *)0x00789A90;
Sprite *g_UNUSED_SPRITES_VAR04 = (Sprite *)0x00792160;
Sprite *g_UNUSED_SPRITES_VAR05 = (Sprite *)0x0075B0C8;
Sprite *g_UNUSED_SPRITES_VAR06 = (Sprite *)0x0075B8C8;
Sprite *g_UNUSED_SPRITES_VAR07 = (Sprite *)0x00787ED0;
Sprite *g_UNUSED_SPRITES_VAR08 = (Sprite *)0x007ACE80;
Sprite *g_UNUSED_SPRITES_VAR09 = (Sprite *)0x007920A8;
Sprite *g_UNUSED_SPRITES_VAR10 = (Sprite *)0x007ACF08;
Sprite *g_UNUSED_SPRITE_VAR75 = (Sprite *)0x007794A8;
Sprite *g_UNUSED_SPRITE_VAR76 = (Sprite *)0x00779670;
Sprite *g_UNUSED_SPRITE_VAR77 = (Sprite *)0x007AB430;
Sprite *g_UNUSED_SPRITE_VAR78 = (Sprite *)0x00798CA0;
Sprite *g_UNUSED_SPRITE_VAR79 = (Sprite *)0x0078E040;
Sprite *g_UNUSED_SPRITE_VAR80 = (Sprite *)0x007A67F0;
Sprite *g_UNUSED_SPRITE_VAR81 = (Sprite *)0x00779618;
Sprite *g_UNUSED_SPRITE_VAR82 = (Sprite *)0x007ACB58;
Sprite *g_UNUSED_SPRITE_VAR83 = (Sprite *)0x007ACC20;
Sprite *g_BASEWIN_SPRITES = (Sprite *)0x0077A5C8;
Wave *g_MAININTERFACE_WAVE = (Wave *)0x007D38B8;
Font *g_JACKAL_FONT = (Font *)0x007D3948;
Sprite *g_IFACE_GREEN_RIGHT_ARROW_SPRITE = (Sprite *)0x007F67C8;
Wave *g_MULTIWIN_WAVE = (Wave *)0x007FFF00;
ButtonGroup *g_PREFWIN_BUTTONGROUP = (ButtonGroup *)0x008577F0;
Buffer *g_VEHDRAW_BUFFER = (Buffer *)0x008CC298;
Sprite *g_CURSOR_SPRITES = (Sprite *)0x0093AA70;
FontQueue *g_FONTQUEUE_VAL2 = (FontQueue *)0x0093FB88;
FontQueue *g_FONTQUEUE_VAL1 = (FontQueue *)0x0093FAE8;
Wave *g_TOP_MENU_WAVE = (Wave *)0x00945780;
Font *g_FONTS = (Font *)0x0093FC58;
Wave *g_CRASH_LANDING_WAVE = (Wave *)0x00945E08;
Wave *g_WAVE_GENERAL = (Wave *)0x00945ED0;
Buffer *g_BUFFER = (Buffer *)0x009B6080;
TextIndex *TxtIndexGlobal = (TextIndex *)0x009B7D08;

/*
Purpose: Atexit teardown thunk for g_ALPHAMENU_WAVE.
Original Offset: 00402F30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_alphamenu_wave() {
    (ORIGINAL(g_ALPHAMENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR02.
Original Offset: 00404460
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var02() {
    g_UNUSED_SPRITE_VAR02->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR11.
Original Offset: 00404490
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var11() {
    g_UNUSED_SPRITE_VAR11->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR06.
Original Offset: 004044C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var06() {
    g_UNUSED_SPRITE_VAR06->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR09.
Original Offset: 004044F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var09() {
    g_UNUSED_SPRITE_VAR09->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR21.
Original Offset: 00404520
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var21() {
    g_UNUSED_SPRITE_VAR21->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR05.
Original Offset: 00404550
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var05() {
    g_UNUSED_SPRITE_VAR05->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR08.
Original Offset: 00404580
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var08() {
    g_UNUSED_SPRITE_VAR08->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR04.
Original Offset: 004045B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var04() {
    g_UNUSED_SPRITE_VAR04->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR01.
Original Offset: 004045E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var01() {
    g_UNUSED_SPRITE_VAR01->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR18.
Original Offset: 00404610
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var18() {
    g_UNUSED_SPRITE_VAR18->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR03.
Original Offset: 00404640
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var03() {
    g_UNUSED_SPRITE_VAR03->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR20.
Original Offset: 00404670
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var20() {
    g_UNUSED_SPRITE_VAR20->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR16.
Original Offset: 004046A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var16() {
    g_UNUSED_SPRITE_VAR16->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR14.
Original Offset: 004046D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var14() {
    g_UNUSED_SPRITE_VAR14->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR22.
Original Offset: 00404700
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var22() {
    g_UNUSED_SPRITE_VAR22->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR10.
Original Offset: 00404730
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var10() {
    g_UNUSED_SPRITE_VAR10->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR15.
Original Offset: 00404760
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var15() {
    g_UNUSED_SPRITE_VAR15->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR13.
Original Offset: 00404790
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var13() {
    g_UNUSED_SPRITE_VAR13->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR17.
Original Offset: 004047C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var17() {
    g_UNUSED_SPRITE_VAR17->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR19.
Original Offset: 004047F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var19() {
    g_UNUSED_SPRITE_VAR19->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR12.
Original Offset: 00404820
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var12() {
    g_UNUSED_SPRITE_VAR12->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR07.
Original Offset: 00404850
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var07() {
    g_UNUSED_SPRITE_VAR07->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN.
Original Offset: 004083D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006A7628))->*BaseWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_WAVE.
Original Offset: 00408400
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_wave() {
    (ORIGINAL(g_BASEWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_BattleWin.
Original Offset: 004219D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battlewin() {
    g_BattleWin->~BattleWin();
}

/*
Purpose: Atexit teardown thunk for g_COUNCWIN.
Original Offset: 004243C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_councwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006FEC80))->*CouncWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CREDITS_WAVE.
Original Offset: 00428770
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_credits_wave() {
    (ORIGINAL(g_CREDITS_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DATALINK.
Original Offset: 00428FB0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_datalink() {
    (ORIGINAL(reinterpret_cast<void *>(0x00703EA0))->*DatalinkDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN.
Original Offset: 00432870
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0071F2B0))->*DesignWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN_WAVE.
Original Offset: 004328A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin_wave() {
    (ORIGINAL(g_DESIGNWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOPOP.
Original Offset: 0043EFE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplopop() {
    (ORIGINAL(reinterpret_cast<void *>(0x00733990))->*DiploPopDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOWIN.
Original Offset: 00440F20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0073ACD8))->*DiploWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CPU_WAVES.
Original Offset: 00445480
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cpu_waves() {
    VectorDtorIterator(g_CPU_WAVES, 0x6C, 45, WaveElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MENU_UP_WAVE.
Original Offset: 004454C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_up_wave() {
    (ORIGINAL(g_MENU_UP_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_MENU_DOWN_WAVE.
Original Offset: 004454F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_down_wave() {
    (ORIGINAL(g_MENU_DOWN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_SCOOT_WAVE.
Original Offset: 00445520
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scoot_wave() {
    (ORIGINAL(g_SCOOT_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_OK_WAVE.
Original Offset: 00445550
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ok_wave() {
    (ORIGINAL(g_OK_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PASSOVER_WAVE.
Original Offset: 00445580
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_passover_wave() {
    (ORIGINAL(g_PASSOVER_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FX.
Original Offset: 004455B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fx() {
    g_FX->~FX();
}

/*
Purpose: Atexit teardown thunk for g_AMBIENCE.
Original Offset: 004455E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ambience() {
    g_AMBIENCE->~Ambience();
}

/*
Purpose: Atexit teardown thunk for g_FAMEWIN.
Original Offset: 004483F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_famewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0074DAF8))->*FameWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PCX_PARSE_TEMP_BUFFER1.
Original Offset: 0044CC40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pcx_parse_temp_buffer1() {
    g_PCX_PARSE_TEMP_BUFFER1->destroy();
}

/*
Purpose: Atexit teardown thunk for FactionArt.
Original Offset: 0044CC80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_factionart() {
    VectorDtorIterator(FactionArtGlobal, 0x65C, 8, FactionArtElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_CLOSE_X_SPRITES.
Original Offset: 0044CCD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_close_x_sprites() {
    VectorDtorIterator(g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES1.
Original Offset: 0044CD20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites1() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES1, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES2.
Original Offset: 0044CD70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites2() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES2, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES3.
Original Offset: 0044CDC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites3() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES3, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES4.
Original Offset: 0044CE10
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites4() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES4, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES5.
Original Offset: 0044CE60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites5() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES5, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES6.
Original Offset: 0044CEB0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites6() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES6, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES7.
Original Offset: 0044CF00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites7() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES7, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES8.
Original Offset: 0044CF50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites8() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES8, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITE_BUFFERS.
Original Offset: 0044CFA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprite_buffers() {
    VectorDtorIterator(g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, BufferElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE.
Original Offset: 0044CFE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_left_sprite() {
    g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE.
Original Offset: 0044D010
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_right_sprite() {
    g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE.
Original Offset: 0044D040
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_left_sprite() {
    g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE.
Original Offset: 0044D070
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_right_sprite() {
    g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_MID_SPRITE.
Original Offset: 0044D0A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_mid_sprite() {
    g_IFACE_STD_POPUPS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_MID_SPRITE.
Original Offset: 0044D0D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_mid_sprite() {
    g_IFACE_STD_POPUPS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_LEFT_SPRITE.
Original Offset: 0044D100
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_left_sprite() {
    g_IFACE_STD_POPUPS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE.
Original Offset: 0044D130
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_right_sprite() {
    g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MIDDLE_BUFFER.
Original Offset: 0044D160
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_middle_buffer() {
    g_IFACE_STD_POPUPS_MIDDLE_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR1.
Original Offset: 0044D190
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var1() {
    g_UNUSED_CAVIARDATA_VAR1->close();
}

/*
Purpose: Atexit teardown thunk for g_SSF_CAVIARDATA.
Original Offset: 0044D1C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssf_caviardata() {
    g_SSF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SDP_CAVIARDATA.
Original Offset: 0044D1F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sdp_caviardata() {
    g_SDP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAS_CAVIARDATA.
Original Offset: 0044D220
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sas_caviardata() {
    g_SAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCD_CAVIARDATA.
Original Offset: 0044D250
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scd_caviardata() {
    g_SCD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCJ_CAVIARDATA.
Original Offset: 0044D280
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scj_caviardata() {
    g_SCJ_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAGS_CAVIARDATA.
Original Offset: 0044D2B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sags_caviardata() {
    g_SAGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFT_CAVIARDATA.
Original Offset: 0044D2E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sft_caviardata() {
    g_SFT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA1.
Original Offset: 0044D310
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata1() {
    g_VHR_CAVIARDATA1->close();
}

/*
Purpose: Atexit teardown thunk for g_SHT_CAVIARDATA.
Original Offset: 0044D340
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sht_caviardata() {
    g_SHT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SRB_CAVIARDATA.
Original Offset: 0044D370
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_srb_caviardata() {
    g_SRB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASAS_CAVIARDATA.
Original Offset: 0044D3A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asas_caviardata() {
    g_ASAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESLASER_CAVIARDATA.
Original Offset: 0044D3D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reslaser_caviardata() {
    g_RESLASER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESBOLT_CAVIARDATA.
Original Offset: 0044D400
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resbolt_caviardata() {
    g_RESBOLT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_FUNLOAD_CAVIARDATA.
Original Offset: 0044D430
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_funload_caviardata() {
    g_FUNLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_TECLOAD_CAVIARDATA.
Original Offset: 0044D460
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tecload_caviardata() {
    g_TECLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_DISSWAVE_CAVIARDATA.
Original Offset: 0044D490
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_disswave_caviardata() {
    g_SP_DISSWAVE_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_MARINED_CAVIARDATA.
Original Offset: 0044D4C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_marined_caviardata() {
    g_SP_MARINED_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_NANOO_CAVIARDATA.
Original Offset: 0044D4F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_nanoo_caviardata() {
    g_SP_NANOO_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_SOPORIFIC_CAVIARDATA.
Original Offset: 0044D520
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_soporific_caviardata() {
    g_SP_SOPORIFIC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA01_CAVIARDATA.
Original Offset: 0044D550
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa01_caviardata() {
    g_AA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_ROVER_CAVIARDATA.
Original Offset: 0044D580
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_rover_caviardata() {
    g_AA_ROVER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_WING_CAVIARDATA.
Original Offset: 0044D5C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_wing_caviardata() {
    VectorDtorIterator(g_AA_WING_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ACP_CAVIARDATA.
Original Offset: 0044D610
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acp_caviardata() {
    VectorDtorIterator(g_ACP_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AX_CAVIARDATA.
Original Offset: 0044D650
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ax_caviardata() {
    g_AX_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_CAVIARDATA.
Original Offset: 0044D680
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_caviardata() {
    g_AA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ACOLPOD_CAVIARDATA.
Original Offset: 0044D6B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acolpod_caviardata() {
    g_ACOLPOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AT_CAVIARDATA.
Original Offset: 0044D6E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_at_caviardata() {
    g_AT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VTA_CAVIARDATA.
Original Offset: 0044D710
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vta_caviardata() {
    g_VTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ATP_CAVIARDATA.
Original Offset: 0044D740
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_atp_caviardata() {
    g_ATP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SSFA_CAVIARDATA.
Original Offset: 0044D770
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssfa_caviardata() {
    g_SSFA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFTA_CAVIARDATA.
Original Offset: 0044D7A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sfta_caviardata() {
    g_SFTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW00_CAVIARDATA.
Original Offset: 0044D7D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw00_caviardata() {
    g_VW00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAWL_CAVIARDATA.
Original Offset: 0044D800
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptawl_caviardata() {
    g_VIPTAWL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTASGN_CAVIARDATA.
Original Offset: 0044D830
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptasgn_caviardata() {
    g_VIPTASGN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAPSI_CAVIARDATA.
Original Offset: 0044D860
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptapsi_caviardata() {
    g_VIPTAPSI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTR_CAVIARDATA.
Original Offset: 0044D8A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptr_caviardata() {
    VectorDtorIterator(g_VIPTR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VIPTA_CAVIARDATA.
Original Offset: 0044D8F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vipta_caviardata() {
    VectorDtorIterator(g_VIPTA_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_PTMOD_CAVIARDATA.
Original Offset: 0044D930
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ptmod_caviardata() {
    g_PTMOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VB_CAVIARDATA.
Original Offset: 0044D960
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vb_caviardata() {
    g_VB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VBP_CAVIARDATA.
Original Offset: 0044D990
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vbp_caviardata() {
    g_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMC_CAVIARDATA.
Original Offset: 0044D9C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmc_caviardata() {
    g_VGMC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMCP_CAVIARDATA.
Original Offset: 0044D9F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmcp_caviardata() {
    g_VGMCP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VLIGHTS_CAVIARDATA.
Original Offset: 0044DA20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vlights_caviardata() {
    g_VLIGHTS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPT_CAVIARDATA.
Original Offset: 0044DA50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpt_caviardata() {
    g_VPT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_A_CAVIARDATA.
Original Offset: 0044DA80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_a_caviardata() {
    g_A_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APWALL_CAVIARDATA.
Original Offset: 0044DAB0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apwall_caviardata() {
    g_APWALL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASGEN_CAVIARDATA.
Original Offset: 0044DAE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asgen_caviardata() {
    g_ASGEN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APSID_CAVIARDATA.
Original Offset: 0044DB10
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apsid_caviardata() {
    g_APSID_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VA01_CAVIARDATA.
Original Offset: 0044DB40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_va01_caviardata() {
    g_VA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VR_CAVIARDATA.
Original Offset: 0044DB80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vr_caviardata() {
    VectorDtorIterator(g_VR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA2.
Original Offset: 0044DBC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata2() {
    g_VHR_CAVIARDATA2->close();
}

/*
Purpose: Atexit teardown thunk for g_VRC_CAVIARDATA.
Original Offset: 0044DC00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vrc_caviardata() {
    VectorDtorIterator(g_VRC_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VI_CAVIARDATA.
Original Offset: 0044DC40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vi_caviardata() {
    g_VI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMT_CAVIARDATA.
Original Offset: 0044DC70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmt_caviardata() {
    g_VGMT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMTP_CAVIARDATA.
Original Offset: 0044DCA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmtp_caviardata() {
    g_VGMTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR2.
Original Offset: 0044DCD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var2() {
    g_UNUSED_CAVIARDATA_VAR2->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNTU_CAVIARDATA.
Original Offset: 0044DD00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntu_caviardata() {
    g_VWNTU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VT_CAVIARDATA.
Original Offset: 0044DD30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vt_caviardata() {
    g_VT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROP_CAVIARDATA.
Original Offset: 0044DD60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_drop_caviardata() {
    g_DROP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROPLET_CAVIARDATA.
Original Offset: 0044DD90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_droplet_caviardata() {
    g_DROPLET_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCL_CAVIARDATA.
Original Offset: 0044DDC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcl_caviardata() {
    g_VCL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR3.
Original Offset: 0044DE00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var3() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCLT00_CAVIARDATA.
Original Offset: 0044DE40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vclt00_caviardata() {
    g_VCLT00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHT_VBP_CAVIARDATA.
Original Offset: 0044DE70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vht_vbp_caviardata() {
    g_VHT_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTP_CAVIARDATA.
Original Offset: 0044DEA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhtp_caviardata() {
    g_VHTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTA0_CAVIARDATA.
Original Offset: 0044DEE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhta0_caviardata() {
    VectorDtorIterator(g_VHTA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHTTP_CAVIARDATA.
Original Offset: 0044DF20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttp_caviardata() {
    g_VHTTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTTPA0_CAVIARDATA.
Original Offset: 0044DF60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttpa0_caviardata() {
    VectorDtorIterator(g_VHTTPA0_CAVIARDATA, 0xC, 3, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSP_CAVIARDATA.
Original Offset: 0044DFA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsp_caviardata() {
    g_VSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPA0_CAVIARDATA.
Original Offset: 0044DFE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vspa0_caviardata() {
    VectorDtorIterator(g_VSPA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSPTF_CAVIARDATA.
Original Offset: 0044E020
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptf_caviardata() {
    g_VSPTF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPTB_CAVIARDATA.
Original Offset: 0044E050
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptb_caviardata() {
    g_VSPTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VFL_CAVIARDATA.
Original Offset: 0044E080
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vfl_caviardata() {
    g_VFL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGS_CAVIARDATA.
Original Offset: 0044E0B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgs_caviardata() {
    g_VGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGSP_CAVIARDATA.
Original Offset: 0044E0E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgsp_caviardata() {
    g_VGSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJTP_CAVIARDATA.
Original Offset: 0044E110
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjtp_caviardata() {
    g_VJTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJT0_CAVIARDATA.
Original Offset: 0044E150
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjt0_caviardata() {
    VectorDtorIterator(g_VJT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCU_CAVIARDATA.
Original Offset: 0044E190
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcu_caviardata() {
    g_VCU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUP_CAVIARDATA.
Original Offset: 0044E1C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcup_caviardata() {
    g_VCUP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUA0_CAVIARDATA.
Original Offset: 0044E200
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcua0_caviardata() {
    VectorDtorIterator(g_VCUA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCUW_CAVIARDATA.
Original Offset: 0044E240
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcuw_caviardata() {
    g_VCUW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT_CAVIARDATA.
Original Offset: 0044E270
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct_caviardata() {
    g_VCT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTP_CAVIARDATA.
Original Offset: 0044E2A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctp_caviardata() {
    g_VCTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTB_CAVIARDATA.
Original Offset: 0044E2D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctb_caviardata() {
    g_VCTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT0_CAVIARDATA.
Original Offset: 0044E310
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct0_caviardata() {
    VectorDtorIterator(g_VCT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VWNTT_CAVIARDATA.
Original Offset: 0044E350
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntt_caviardata() {
    g_VWNTT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNST_CAVIARDATA.
Original Offset: 0044E380
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnst_caviardata() {
    g_VWNST_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNAA_CAVIARDATA.
Original Offset: 0044E3B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnaa_caviardata() {
    g_VWNAA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW_CAVIARDATA.
Original Offset: 0044E3F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw_caviardata() {
    VectorDtorIterator(g_VW_CAVIARDATA, 0xC, 16, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VM_CAVIARDATA.
Original Offset: 0044E430
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm_caviardata() {
    g_VM_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VM13_CAVIARDATA.
Original Offset: 0044E460
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm13_caviardata() {
    g_VM13_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPBR0_CAVIARDATA.
Original Offset: 0044E4A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpbr0_caviardata() {
    VectorDtorIterator(g_VPBR0_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR4.
Original Offset: 0044E4F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var4() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NW_CAVIARDATA.
Original Offset: 0044E530
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nw_caviardata() {
    g_NW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NI_CAVIARDATA.
Original Offset: 0044E560
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ni_caviardata() {
    g_NI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NLC_CAVIARDATA.
Original Offset: 0044E590
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nlc_caviardata() {
    g_NLC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR5.
Original Offset: 0044E5D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var5() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RADIUS1_TEXTURE.
Original Offset: 0044E610
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius1_texture() {
    g_RADIUS1_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_RADIUS2_TEXTURE.
Original Offset: 0044E640
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius2_texture() {
    g_RADIUS2_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_ROCKY_TEXTURES.
Original Offset: 0044E680
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rocky_textures() {
    VectorDtorIterator(g_ROCKY_TEXTURES, 0x70, 4, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_OCEAN_TEXTURES.
Original Offset: 0044E6D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ocean_textures() {
    VectorDtorIterator(g_OCEAN_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAT_ARID_LAND_TEXTURE.
Original Offset: 0044E710
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flat_arid_land_texture() {
    g_FLAT_ARID_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_MOIST_LAND_TEXTURES.
Original Offset: 0044E750
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_moist_land_textures() {
    VectorDtorIterator(g_MOIST_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINY_LAND_TEXTURES.
Original Offset: 0044E7A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainy_land_textures() {
    VectorDtorIterator(g_RAINY_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_JUNGLE_LAND_TEXTURES.
Original Offset: 0044E7F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jungle_land_textures() {
    VectorDtorIterator(g_JUNGLE_LAND_TEXTURES, 0x70, 15, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_DUNE_LAND_TEXTURE.
Original Offset: 0044E830
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_dune_land_texture() {
    g_DUNE_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_SUNNY_MESA_TEXTURES.
Original Offset: 0044E870
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sunny_mesa_textures() {
    VectorDtorIterator(g_SUNNY_MESA_TEXTURES, 0x70, 8, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURES.
Original Offset: 0044E8C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_textures() {
    VectorDtorIterator(g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ROAD_TEXTURES.
Original Offset: 0044E910
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_road_textures() {
    VectorDtorIterator(g_ROAD_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MAGTUBE_TEXTURES.
Original Offset: 0044E960
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_magtube_textures() {
    VectorDtorIterator(g_MAGTUBE_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RIVER_TEXTURES.
Original Offset: 0044E9B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_river_textures() {
    VectorDtorIterator(g_RIVER_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MOUNT_PLANET_TEXTURES.
Original Offset: 0044EA00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_mount_planet_textures() {
    VectorDtorIterator(g_MOUNT_PLANET_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GARLAND_CRATER_TEXTURES.
Original Offset: 0044EA50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_garland_crater_textures() {
    VectorDtorIterator(g_GARLAND_CRATER_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FUNGUS_TEXTURES.
Original Offset: 0044EAA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fungus_textures() {
    VectorDtorIterator(g_FUNGUS_TEXTURES, 0x70, 30, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FARM_TEXTURES.
Original Offset: 0044EAF0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_farm_textures() {
    VectorDtorIterator(g_FARM_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOREST_TEXTURES.
Original Offset: 0044EB40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_forest_textures() {
    VectorDtorIterator(g_FOREST_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURE.
Original Offset: 0044EB80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_texture() {
    g_RAINFALL_SINGLE_TILE_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_TER1_WHITE_ORG_YEL_TILE_SPRITES.
Original Offset: 0044EBC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_white_org_yel_tile_sprites() {
    VectorDtorIterator(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOTTOM_LEFT_TILE_SPRITES.
Original Offset: 0044EC10
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bottom_left_tile_sprites() {
    VectorDtorIterator(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES2.
Original Offset: 0044EC60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites2() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES2, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINE_SPRITE.
Original Offset: 0044ECA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mine_sprite() {
    g_TER1_MINE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOLAR_COLLECTOR_SPRITE.
Original Offset: 0044ECD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_solar_collector_sprite() {
    g_TER1_SOLAR_COLLECTOR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TIDAL_HARNESS_SPRITE.
Original Offset: 0044ED00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tidal_harness_sprite() {
    g_TER1_TIDAL_HARNESS_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINING_PLATFORM_SPRITE.
Original Offset: 0044ED30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mining_platform_sprite() {
    g_TER1_MINING_PLATFORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TUT_BLANK_SPRITE.
Original Offset: 0044ED60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tut_blank_sprite() {
    g_TER1_TUT_BLANK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_KELP_FARM_SPRITE.
Original Offset: 0044ED90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_kelp_farm_sprite() {
    g_TER1_KELP_FARM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_CONDENSER_SPRITE.
Original Offset: 0044EDC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_condenser_sprite() {
    g_TER1_CONDENSER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_ECHELON_MIRROR_SPRITE.
Original Offset: 0044EDF0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_echelon_mirror_sprite() {
    g_TER1_ECHELON_MIRROR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_SPRITE.
Original Offset: 0044EE20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_sprite() {
    g_TER1_BOREHOLE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_CLUSTER_SPRITE.
Original Offset: 0044EE50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_cluster_sprite() {
    g_TER1_BOREHOLE_CLUSTER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MANIFOLD_NEXUS_SPRITES.
Original Offset: 0044EE90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_manifold_nexus_sprites() {
    VectorDtorIterator(g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_SPRITES.
Original Offset: 0044EEE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES.
Original Offset: 0044EF30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_alt_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOSSIL_FIELD_RIDGE_SPRITES.
Original Offset: 0044EF80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fossil_field_ridge_sprites() {
    VectorDtorIterator(g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES1.
Original Offset: 0044EFD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites1() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES1, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_FARM_SPRITES.
Original Offset: 0044F020
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_farm_sprites() {
    VectorDtorIterator(g_TER1_FARM_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOIL_ENRICHER_SPRITES.
Original Offset: 0044F070
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_soil_enricher_sprites() {
    VectorDtorIterator(g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SEA_LAND_RESOURCE_SPRITES.
Original Offset: 0044F0C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sea_land_resource_sprites() {
    VectorDtorIterator(g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_LANDMARK_RESOURCE_SPRITES.
Original Offset: 0044F110
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_landmark_resource_sprites() {
    VectorDtorIterator(g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GLOW_SPRITES.
Original Offset: 0044F160
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_glow_sprites() {
    VectorDtorIterator(g_GLOW_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNITY_POD_SPRITES.
Original Offset: 0044F1B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unity_pod_sprites() {
    VectorDtorIterator(g_TER1_UNITY_POD_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MONOLITH_SPRITE.
Original Offset: 0044F1F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_monolith_sprite() {
    g_TER1_MONOLITH_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BUNKER_SPRITE.
Original Offset: 0044F220
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bunker_sprite() {
    g_TER1_BUNKER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_AIRBASE_SPRITE.
Original Offset: 0044F250
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_airbase_sprite() {
    g_TER1_AIRBASE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SENSOR_ARRAY_SPRITE.
Original Offset: 0044F280
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sensor_array_sprite() {
    g_TER1_SENSOR_ARRAY_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_DOUBLE_TILE_SPRITES.
Original Offset: 0044F2C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_double_tile_sprites() {
    VectorDtorIterator(g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VEH_SPRITES.
Original Offset: 0044F310
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_veh_sprites() {
    VectorDtorIterator(g_VEH_SPRITES, 0x2C, 152, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAGS_VEH_SPRITES.
Original Offset: 0044F360
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flags_veh_sprites() {
    VectorDtorIterator(g_FLAGS_VEH_SPRITES, 0x2C, 112, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICONS_GENERAL_SPRITES.
Original Offset: 0044F3B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icons_general_sprites() {
    VectorDtorIterator(g_ICONS_GENERAL_SPRITES, 0x2C, 16, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RESOURCE_ICON_SPRITES.
Original Offset: 0044F400
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resource_icon_sprites() {
    VectorDtorIterator(g_RESOURCE_ICON_SPRITES, 0x2C, 32, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_LG_CURSOR_SPRITES.
Original Offset: 0044F450
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_LG_CURSOR_SPRITES.
Original Offset: 0044F4A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_SM_CURSOR_SPRITES.
Original Offset: 0044F4F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_SM_CURSOR_SPRITES.
Original Offset: 0044F540
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_LG_CURSOR_SPRITES.
Original Offset: 0044F590
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_LG_CURSOR_SPRITES.
Original Offset: 0044F5E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_SM_CURSOR_SPRITES.
Original Offset: 0044F630
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_SM_CURSOR_SPRITES.
Original Offset: 0044F680
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_ALIEN_HEAD_ICON_SPRITE.
Original Offset: 0044F6C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_alien_head_icon_sprite() {
    g_RED_ALIEN_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_SILVER_MENU_ICON_SPRITES.
Original Offset: 0044F700
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_menu_icon_sprites() {
    VectorDtorIterator(g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SILVER_CHECKBOX_ICON_SPRITES.
Original Offset: 0044F750
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_checkbox_icon_sprites() {
    VectorDtorIterator(g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_MALE_HEAD_ICON_SPRITE.
Original Offset: 0044F790
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_male_head_icon_sprite() {
    g_RED_MALE_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_NULL_RESOURCE_ICON_SPRITE.
Original Offset: 0044F7C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_null_resource_icon_sprite() {
    g_NULL_RESOURCE_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_PEACE_SIGN_SPRITES.
Original Offset: 0044F800
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_peace_sign_sprites() {
    VectorDtorIterator(g_PEACE_SIGN_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICON_TILE_SQUARE_SPRITE.
Original Offset: 0044F840
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icon_tile_square_sprite() {
    g_ICON_TILE_SQUARE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_XI_BOOM_VEH_SPRITES.
Original Offset: 0044F880
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xi_boom_veh_sprites() {
    VectorDtorIterator(g_XI_BOOM_VEH_SPRITES, 0x2C, 144, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_XF_BOOM_VEH_SPRITES.
Original Offset: 0044F8D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xf_boom_veh_sprites() {
    VectorDtorIterator(g_XF_BOOM_VEH_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_MIND_WORM_SPRITE.
Original Offset: 0044F910
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_mind_worm_sprite() {
    g_BATTLE_MIND_WORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_ISLE_DEEP_SPRITE.
Original Offset: 0044F940
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_isle_deep_sprite() {
    g_BATTLE_ISLE_DEEP_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_LOCUSTS_CHIRON_SPRITE.
Original Offset: 0044F970
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_locusts_chiron_sprite() {
    g_BATTLE_LOCUSTS_CHIRON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_FUNGAL_TOWER_SPRITE.
Original Offset: 0044F9A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_fungal_tower_sprite() {
    g_BATTLE_FUNGAL_TOWER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SPORE_LAUNCHER_SPRITE.
Original Offset: 0044F9D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_spore_launcher_sprite() {
    g_BATTLE_SPORE_LAUNCHER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SEALURK_SPRITE.
Original Offset: 0044FA00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_sealurk_sprite() {
    g_BATTLE_SEALURK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TECH_ICON_SPRITES.
Original Offset: 0044FA40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tech_icon_sprites() {
    VectorDtorIterator(g_TECH_ICON_SPRITES, 0x2C, 89, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FACILITY_ICON_SPRITES.
Original Offset: 0044FA90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_facility_icon_sprites() {
    VectorDtorIterator(g_FACILITY_ICON_SPRITES, 0x2C, 70, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SECRET_PROJECT_ICON_SPRITES.
Original Offset: 0044FAE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_secret_project_icon_sprites() {
    VectorDtorIterator(g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_MP_COMBO_ARROW_SPRITES.
Original Offset: 0044FB30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_mp_combo_arrow_sprites() {
    VectorDtorIterator(g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_ARROW_ICON_SPRITES.
Original Offset: 0044FB80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES.
Original Offset: 0044FBD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_FILLER_ICON_SPRITES.
Original Offset: 0044FC10
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_filler_icon_sprites() {
    g_SCROLL_BAR_FILLER_ICON_SPRITES->close();
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE.
Original Offset: 0044FC40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_filler_icon_sprite() {
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_LOCK_SPRITES.
Original Offset: 0044FC80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_lock_sprites() {
    VectorDtorIterator(g_IFACE_LOCK_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR01.
Original Offset: 0044FCD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var01() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR01, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE.
Original Offset: 0044FD10
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE.
Original Offset: 0044FD40
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE.
Original Offset: 0044FD70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE.
Original Offset: 0044FDA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE.
Original Offset: 0044FDD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE.
Original Offset: 0044FE00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE.
Original Offset: 0044FE30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE.
Original Offset: 0044FE60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE.
Original Offset: 0044FE90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE.
Original Offset: 0044FEC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE.
Original Offset: 0044FEF0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR23.
Original Offset: 0044FF20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var23() {
    g_UNUSED_SPRITE_VAR23->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR24.
Original Offset: 0044FF50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var24() {
    g_UNUSED_SPRITE_VAR24->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR25.
Original Offset: 0044FF80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var25() {
    g_UNUSED_SPRITE_VAR25->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR26.
Original Offset: 0044FFB0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var26() {
    g_UNUSED_SPRITE_VAR26->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR27.
Original Offset: 0044FFE0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var27() {
    g_UNUSED_SPRITE_VAR27->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR28.
Original Offset: 00450010
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var28() {
    g_UNUSED_SPRITE_VAR28->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR29.
Original Offset: 00450040
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var29() {
    g_UNUSED_SPRITE_VAR29->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR30.
Original Offset: 00450070
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var30() {
    g_UNUSED_SPRITE_VAR30->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR31.
Original Offset: 004500A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var31() {
    g_UNUSED_SPRITE_VAR31->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR32.
Original Offset: 004500D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var32() {
    g_UNUSED_SPRITE_VAR32->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR33.
Original Offset: 00450100
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var33() {
    g_UNUSED_SPRITE_VAR33->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR34.
Original Offset: 00450130
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var34() {
    g_UNUSED_SPRITE_VAR34->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR35.
Original Offset: 00450160
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var35() {
    g_UNUSED_SPRITE_VAR35->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR36.
Original Offset: 00450190
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var36() {
    g_UNUSED_SPRITE_VAR36->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR37.
Original Offset: 004501C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var37() {
    g_UNUSED_SPRITE_VAR37->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR38.
Original Offset: 004501F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var38() {
    g_UNUSED_SPRITE_VAR38->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR39.
Original Offset: 00450220
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var39() {
    g_UNUSED_SPRITE_VAR39->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR40.
Original Offset: 00450250
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var40() {
    g_UNUSED_SPRITE_VAR40->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR41.
Original Offset: 00450280
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var41() {
    g_UNUSED_SPRITE_VAR41->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR42.
Original Offset: 004502B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var42() {
    g_UNUSED_SPRITE_VAR42->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR43.
Original Offset: 004502E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var43() {
    g_UNUSED_SPRITE_VAR43->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR44.
Original Offset: 00450310
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var44() {
    g_UNUSED_SPRITE_VAR44->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR45.
Original Offset: 00450340
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var45() {
    g_UNUSED_SPRITE_VAR45->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR46.
Original Offset: 00450370
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var46() {
    g_UNUSED_SPRITE_VAR46->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_TECH_TREE_ARROW_SPRITES.
Original Offset: 004503B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_tech_tree_arrow_sprites() {
    VectorDtorIterator(g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR47.
Original Offset: 004503F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var47() {
    g_UNUSED_SPRITE_VAR47->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR48.
Original Offset: 00450420
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var48() {
    g_UNUSED_SPRITE_VAR48->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR49.
Original Offset: 00450450
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var49() {
    g_UNUSED_SPRITE_VAR49->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR50.
Original Offset: 00450480
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var50() {
    g_UNUSED_SPRITE_VAR50->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR51.
Original Offset: 004504B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var51() {
    g_UNUSED_SPRITE_VAR51->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR52.
Original Offset: 004504E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var52() {
    g_UNUSED_SPRITE_VAR52->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR53.
Original Offset: 00450510
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var53() {
    g_UNUSED_SPRITE_VAR53->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR54.
Original Offset: 00450540
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var54() {
    g_UNUSED_SPRITE_VAR54->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR55.
Original Offset: 00450570
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var55() {
    g_UNUSED_SPRITE_VAR55->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR56.
Original Offset: 004505A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var56() {
    g_UNUSED_SPRITE_VAR56->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR57.
Original Offset: 004505D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var57() {
    g_UNUSED_SPRITE_VAR57->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR58.
Original Offset: 00450600
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var58() {
    g_UNUSED_SPRITE_VAR58->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR59.
Original Offset: 00450630
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var59() {
    g_UNUSED_SPRITE_VAR59->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR60.
Original Offset: 00450660
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var60() {
    g_UNUSED_SPRITE_VAR60->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR61.
Original Offset: 00450690
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var61() {
    g_UNUSED_SPRITE_VAR61->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR62.
Original Offset: 004506C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var62() {
    g_UNUSED_SPRITE_VAR62->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR63.
Original Offset: 004506F0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var63() {
    g_UNUSED_SPRITE_VAR63->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR64.
Original Offset: 00450720
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var64() {
    g_UNUSED_SPRITE_VAR64->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR65.
Original Offset: 00450750
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var65() {
    g_UNUSED_SPRITE_VAR65->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR66.
Original Offset: 00450780
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var66() {
    g_UNUSED_SPRITE_VAR66->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR67.
Original Offset: 004507B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var67() {
    g_UNUSED_SPRITE_VAR67->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR68.
Original Offset: 004507E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var68() {
    g_UNUSED_SPRITE_VAR68->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR69.
Original Offset: 00450810
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var69() {
    g_UNUSED_SPRITE_VAR69->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR70.
Original Offset: 00450840
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var70() {
    g_UNUSED_SPRITE_VAR70->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR71.
Original Offset: 00450870
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var71() {
    g_UNUSED_SPRITE_VAR71->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR72.
Original Offset: 004508A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var72() {
    g_UNUSED_SPRITE_VAR72->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR73.
Original Offset: 004508D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var73() {
    g_UNUSED_SPRITE_VAR73->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR74.
Original Offset: 00450900
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var74() {
    g_UNUSED_SPRITE_VAR74->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR02.
Original Offset: 00450940
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var02() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR02, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR03.
Original Offset: 00450990
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var03() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR03, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR04.
Original Offset: 004509E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var04() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR04, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR05.
Original Offset: 00450A30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var05() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR05, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR06.
Original Offset: 00450A80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var06() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR06, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR07.
Original Offset: 00450AD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var07() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR07, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR08.
Original Offset: 00450B20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var08() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR08, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR09.
Original Offset: 00450B70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var09() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR09, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR10.
Original Offset: 00450BC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var10() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR10, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR75.
Original Offset: 00450C00
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var75() {
    g_UNUSED_SPRITE_VAR75->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR76.
Original Offset: 00450C30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var76() {
    g_UNUSED_SPRITE_VAR76->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR77.
Original Offset: 00450C60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var77() {
    g_UNUSED_SPRITE_VAR77->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR78.
Original Offset: 00450C90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var78() {
    g_UNUSED_SPRITE_VAR78->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR79.
Original Offset: 00450CC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var79() {
    g_UNUSED_SPRITE_VAR79->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR80.
Original Offset: 00450CF0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var80() {
    g_UNUSED_SPRITE_VAR80->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR81.
Original Offset: 00450D20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var81() {
    g_UNUSED_SPRITE_VAR81->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR82.
Original Offset: 00450D50
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var82() {
    g_UNUSED_SPRITE_VAR82->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR83.
Original Offset: 00450D80
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var83() {
    g_UNUSED_SPRITE_VAR83->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_SPRITES.
Original Offset: 00450DC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_sprites() {
    VectorDtorIterator(g_BASEWIN_SPRITES, 0x2C, 27, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_INFOWIN.
Original Offset: 004562E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_infowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AD2A0))->*InfoWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE.
Original Offset: 00459580
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AE820))->*MainInterfaceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE_WAVE.
Original Offset: 004595B0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface_wave() {
    (ORIGINAL(g_MAININTERFACE_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_JACKAL_FONT.
Original Offset: 0045F940
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jackal_font() {
    g_JACKAL_FONT->~Font();
}

/*
Purpose: Atexit teardown thunk for g_MESSAGEWIN.
Original Offset: 00471340
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_messagewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F67F8))->*MessageWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GREEN_RIGHT_ARROW_SPRITE.
Original Offset: 00471380
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_green_right_arrow_sprite() {
    VectorDtorIterator(g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MONUWIN.
Original Offset: 00472220
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_monuwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F9F58))->*MonuWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN.
Original Offset: 00477E30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FD648))->*MultiWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN_WAVE.
Original Offset: 00477E60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin_wave() {
    (ORIGINAL(g_MULTIWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG1.
Original Offset: 0047A790
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg1() {
    (ORIGINAL(reinterpret_cast<void *>(0x00805338))->*NetMsgDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG2.
Original Offset: 0047A7C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg2() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FFF80))->*NetMsgDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NETWIN.
Original Offset: 0047B010
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0080A6F8))->*NetWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NEWTECHWIN.
Original Offset: 00483870
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_newtechwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00811E40))->*NewTechWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PICKWIN.
Original Offset: 00488770
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pickwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00822718))->*PickWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN.
Original Offset: 0048D510
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008578D8))->*PrefWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN_BUTTONGROUP.
Original Offset: 0048D540
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin_buttongroup() {
    g_PREFWIN_BUTTONGROUP->close();
}

/*
Purpose: Atexit teardown thunk for g_QUAYLEWIN.
Original Offset: 00495190
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_quaylewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00872CB0))->*QuayleWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_REPORTIF.
Original Offset: 00496920
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportif() {
    (ORIGINAL(reinterpret_cast<void *>(0x00885F38))->*ReportIfDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_REPORTWIN.
Original Offset: 00496950
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00876478))->*ReportWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for SocialWinParent.
Original Offset: 004AE9D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_socialwinparent() {
    (ORIGINAL(reinterpret_cast<void *>(0x008A6270))->*SocialWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_STATUSWIN.
Original Offset: 004B3FC0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_statuswin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C5568))->*StatusWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_TUTWIN.
Original Offset: 004BA5E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tutwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C6E68))->*TutWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_CAVIAR.
Original Offset: 004BF700
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_caviar() {
    (ORIGINAL(reinterpret_cast<void *>(0x008CC828))->*CaviarCloseTarget)();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_BUFFER.
Original Offset: 004BF730
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_buffer() {
    g_VEHDRAW_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for g_WORLDWIN.
Original Offset: 004C38D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_worldwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008E9F60))->*WorldWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_DEVICE.
Original Offset: 004C5C70
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D978))->*Wave_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MIDI_DEVICE.
Original Offset: 004C5CA0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_midi_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D950))->*Midi_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_IN_DEVICE.
Original Offset: 004C5CD0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_in_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090DB50))->*Wave_In_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CONSOLE_TIMER.
Original Offset: 0050E9A0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_console_timer() {
    (ORIGINAL(reinterpret_cast<void *>(0x00939E88))->*TimeDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CURSOR_SPRITES.
Original Offset: 0052DAF0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cursor_sprites() {
    VectorDtorIterator(g_CURSOR_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NETDAEMON.
Original Offset: 0052DB30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netdaemon() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093CD90))->*NetDaemonDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL2.
Original Offset: 00559250
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val2() {
    g_FONTQUEUE_VAL2->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL1.
Original Offset: 00559280
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val1() {
    g_FONTQUEUE_VAL1->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_TOP_MENU_WAVE.
Original Offset: 00584D20
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_top_menu_wave() {
    (ORIGINAL(g_TOP_MENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FONTS.
Original Offset: 005882D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fonts() {
    VectorDtorIterator(g_FONTS, 0x28, 48, FontElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CRASH_LANDING_WAVE.
Original Offset: 005AE110
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_crash_landing_wave() {
    (ORIGINAL(g_CRASH_LANDING_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_GENERAL.
Original Offset: 005B9C30
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_general() {
    (ORIGINAL(g_WAVE_GENERAL)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE1.
Original Offset: 005BEC60
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette1() {
    (ORIGINAL(reinterpret_cast<void *>(0x0094C590))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE2.
Original Offset: 005BEC90
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette2() {
    (ORIGINAL(reinterpret_cast<void *>(0x009523A0))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIDEBUG.
Original Offset: 005C97E0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multidebug() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B22F0))->*MultiDebugDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_BUFFER.
Original Offset: 005E37D0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_buffer() {
    g_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for TxtIndex.
Original Offset: 005FD510
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_txtindex() {
    VectorDtorIterator(TxtIndexGlobal, 0x118, 4, TextIndexElementTeardown);
}

/*
Purpose: Atexit teardown thunk for StringTable.
Original Offset: 006168C0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_stringtable() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B90D8))->*StringsDtorTarget)();
}
