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
ORIGINAL: 0x00402F30
// name      ??__Fg_ALPHAMENU_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00402F30-0x00402F3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_alphamenu_wave() {
    (ORIGINAL(g_ALPHAMENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR02.
ORIGINAL: 0x00404460
// name      ??__Fg_UNUSED_SPRITE_VAR02@@YAXXZ
// size      10 bytes
// spans     0x00404460-0x0040446A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var02() {
    g_UNUSED_SPRITE_VAR02->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR11.
ORIGINAL: 0x00404490
// name      ??__Fg_UNUSED_SPRITE_VAR11@@YAXXZ
// size      10 bytes
// spans     0x00404490-0x0040449A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var11() {
    g_UNUSED_SPRITE_VAR11->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR06.
ORIGINAL: 0x004044C0
// name      ??__Fg_UNUSED_SPRITE_VAR06@@YAXXZ
// size      10 bytes
// spans     0x004044C0-0x004044CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var06() {
    g_UNUSED_SPRITE_VAR06->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR09.
ORIGINAL: 0x004044F0
// name      ??__Fg_UNUSED_SPRITE_VAR09@@YAXXZ
// size      10 bytes
// spans     0x004044F0-0x004044FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var09() {
    g_UNUSED_SPRITE_VAR09->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR21.
ORIGINAL: 0x00404520
// name      ??__Fg_UNUSED_SPRITE_VAR21@@YAXXZ
// size      10 bytes
// spans     0x00404520-0x0040452A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var21() {
    g_UNUSED_SPRITE_VAR21->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR05.
ORIGINAL: 0x00404550
// name      ??__Fg_UNUSED_SPRITE_VAR05@@YAXXZ
// size      10 bytes
// spans     0x00404550-0x0040455A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var05() {
    g_UNUSED_SPRITE_VAR05->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR08.
ORIGINAL: 0x00404580
// name      ??__Fg_UNUSED_SPRITE_VAR08@@YAXXZ
// size      10 bytes
// spans     0x00404580-0x0040458A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var08() {
    g_UNUSED_SPRITE_VAR08->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR04.
ORIGINAL: 0x004045B0
// name      ??__Fg_UNUSED_SPRITE_VAR04@@YAXXZ
// size      10 bytes
// spans     0x004045B0-0x004045BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var04() {
    g_UNUSED_SPRITE_VAR04->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR01.
ORIGINAL: 0x004045E0
// name      ??__Fg_UNUSED_SPRITE_VAR01@@YAXXZ
// size      10 bytes
// spans     0x004045E0-0x004045EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var01() {
    g_UNUSED_SPRITE_VAR01->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR18.
ORIGINAL: 0x00404610
// name      ??__Fg_UNUSED_SPRITE_VAR18@@YAXXZ
// size      10 bytes
// spans     0x00404610-0x0040461A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var18() {
    g_UNUSED_SPRITE_VAR18->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR03.
ORIGINAL: 0x00404640
// name      ??__Fg_UNUSED_SPRITE_VAR03@@YAXXZ
// size      10 bytes
// spans     0x00404640-0x0040464A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var03() {
    g_UNUSED_SPRITE_VAR03->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR20.
ORIGINAL: 0x00404670
// name      ??__Fg_UNUSED_SPRITE_VAR20@@YAXXZ
// size      10 bytes
// spans     0x00404670-0x0040467A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var20() {
    g_UNUSED_SPRITE_VAR20->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR16.
ORIGINAL: 0x004046A0
// name      ??__Fg_UNUSED_SPRITE_VAR16@@YAXXZ
// size      10 bytes
// spans     0x004046A0-0x004046AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var16() {
    g_UNUSED_SPRITE_VAR16->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR14.
ORIGINAL: 0x004046D0
// name      ??__Fg_UNUSED_SPRITE_VAR14@@YAXXZ
// size      10 bytes
// spans     0x004046D0-0x004046DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var14() {
    g_UNUSED_SPRITE_VAR14->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR22.
ORIGINAL: 0x00404700
// name      ??__Fg_UNUSED_SPRITE_VAR22@@YAXXZ
// size      10 bytes
// spans     0x00404700-0x0040470A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var22() {
    g_UNUSED_SPRITE_VAR22->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR10.
ORIGINAL: 0x00404730
// name      ??__Fg_UNUSED_SPRITE_VAR10@@YAXXZ
// size      10 bytes
// spans     0x00404730-0x0040473A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var10() {
    g_UNUSED_SPRITE_VAR10->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR15.
ORIGINAL: 0x00404760
// name      ??__Fg_UNUSED_SPRITE_VAR15@@YAXXZ
// size      10 bytes
// spans     0x00404760-0x0040476A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var15() {
    g_UNUSED_SPRITE_VAR15->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR13.
ORIGINAL: 0x00404790
// name      ??__Fg_UNUSED_SPRITE_VAR13@@YAXXZ
// size      10 bytes
// spans     0x00404790-0x0040479A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var13() {
    g_UNUSED_SPRITE_VAR13->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR17.
ORIGINAL: 0x004047C0
// name      ??__Fg_UNUSED_SPRITE_VAR17@@YAXXZ
// size      10 bytes
// spans     0x004047C0-0x004047CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var17() {
    g_UNUSED_SPRITE_VAR17->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR19.
ORIGINAL: 0x004047F0
// name      ??__Fg_UNUSED_SPRITE_VAR19@@YAXXZ
// size      10 bytes
// spans     0x004047F0-0x004047FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var19() {
    g_UNUSED_SPRITE_VAR19->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR12.
ORIGINAL: 0x00404820
// name      ??__Fg_UNUSED_SPRITE_VAR12@@YAXXZ
// size      10 bytes
// spans     0x00404820-0x0040482A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var12() {
    g_UNUSED_SPRITE_VAR12->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR07.
ORIGINAL: 0x00404850
// name      ??__Fg_UNUSED_SPRITE_VAR07@@YAXXZ
// size      10 bytes
// spans     0x00404850-0x0040485A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var07() {
    g_UNUSED_SPRITE_VAR07->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN.
ORIGINAL: 0x004083D0
// name      ??__Fg_BASEWIN@@YAXXZ
// size      10 bytes
// spans     0x004083D0-0x004083DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006A7628))->*BaseWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_WAVE.
ORIGINAL: 0x00408400
// name      ??__Fg_BASEWIN_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00408400-0x0040840A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_wave() {
    (ORIGINAL(g_BASEWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_BattleWin.
ORIGINAL: 0x004219D0
// name      ??__Fg_BattleWin@@YAXXZ
// size      10 bytes
// spans     0x004219D0-0x004219DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battlewin() {
    g_BattleWin->~BattleWin();
}

/*
Purpose: Atexit teardown thunk for g_COUNCWIN.
ORIGINAL: 0x004243C0
// name      ??__Fg_COUNCWIN@@YAXXZ
// size      10 bytes
// spans     0x004243C0-0x004243CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_councwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x006FEC80))->*CouncWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CREDITS_WAVE.
ORIGINAL: 0x00428770
// name      ??__Fg_CREDITS_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00428770-0x0042877A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_credits_wave() {
    (ORIGINAL(g_CREDITS_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DATALINK.
ORIGINAL: 0x00428FB0
// name      ??__Fg_DATALINK@@YAXXZ
// size      10 bytes
// spans     0x00428FB0-0x00428FBA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_datalink() {
    (ORIGINAL(reinterpret_cast<void *>(0x00703EA0))->*DatalinkDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN.
ORIGINAL: 0x00432870
// name      ??__Fg_DESIGNWIN@@YAXXZ
// size      10 bytes
// spans     0x00432870-0x0043287A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0071F2B0))->*DesignWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DESIGNWIN_WAVE.
ORIGINAL: 0x004328A0
// name      ??__Fg_DESIGNWIN_WAVE@@YAXXZ
// size      10 bytes
// spans     0x004328A0-0x004328AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_designwin_wave() {
    (ORIGINAL(g_DESIGNWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOPOP.
ORIGINAL: 0x0043EFE0
// name      ??__Fg_DIPLOPOP@@YAXXZ
// size      10 bytes
// spans     0x0043EFE0-0x0043EFEA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplopop() {
    (ORIGINAL(reinterpret_cast<void *>(0x00733990))->*DiploPopDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_DIPLOWIN.
ORIGINAL: 0x00440F20
// name      ??__Fg_DIPLOWIN@@YAXXZ
// size      10 bytes
// spans     0x00440F20-0x00440F2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_diplowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0073ACD8))->*DiploWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CPU_WAVES.
ORIGINAL: 0x00445480
// name      ??__Fg_CPU_WAVES@@YAXXZ
// size      20 bytes
// spans     0x00445480-0x00445494
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cpu_waves() {
    VectorDtorIterator(g_CPU_WAVES, 0x6C, 45, WaveElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MENU_UP_WAVE.
ORIGINAL: 0x004454C0
// name      ??__Fg_MENU_UP_WAVE@@YAXXZ
// size      10 bytes
// spans     0x004454C0-0x004454CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_up_wave() {
    (ORIGINAL(g_MENU_UP_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_MENU_DOWN_WAVE.
ORIGINAL: 0x004454F0
// name      ??__Fg_MENU_DOWN_WAVE@@YAXXZ
// size      10 bytes
// spans     0x004454F0-0x004454FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_menu_down_wave() {
    (ORIGINAL(g_MENU_DOWN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_SCOOT_WAVE.
ORIGINAL: 0x00445520
// name      ??__Fg_SCOOT_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00445520-0x0044552A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scoot_wave() {
    (ORIGINAL(g_SCOOT_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_OK_WAVE.
ORIGINAL: 0x00445550
// name      ??__Fg_OK_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00445550-0x0044555A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ok_wave() {
    (ORIGINAL(g_OK_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PASSOVER_WAVE.
ORIGINAL: 0x00445580
// name      ??__Fg_PASSOVER_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00445580-0x0044558A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_passover_wave() {
    (ORIGINAL(g_PASSOVER_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FX.
ORIGINAL: 0x004455B0
// name      ??__Fg_FX@@YAXXZ
// size      10 bytes
// spans     0x004455B0-0x004455BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fx() {
    g_FX->~FX();
}

/*
Purpose: Atexit teardown thunk for g_AMBIENCE.
ORIGINAL: 0x004455E0
// name      ??__Fg_AMBIENCE@@YAXXZ
// size      10 bytes
// spans     0x004455E0-0x004455EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ambience() {
    g_AMBIENCE->~Ambience();
}

/*
Purpose: Atexit teardown thunk for g_FAMEWIN.
ORIGINAL: 0x004483F0
// name      ??__Fg_FAMEWIN@@YAXXZ
// size      10 bytes
// spans     0x004483F0-0x004483FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_famewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0074DAF8))->*FameWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PCX_PARSE_TEMP_BUFFER1.
ORIGINAL: 0x0044CC40
// name      ??__Fg_PCX_PARSE_TEMP_BUFFER1@@YAXXZ
// size      10 bytes
// spans     0x0044CC40-0x0044CC4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pcx_parse_temp_buffer1() {
    g_PCX_PARSE_TEMP_BUFFER1->destroy();
}

/*
Purpose: Atexit teardown thunk for FactionArt.
ORIGINAL: 0x0044CC80
// name      ??__FFactionArt@@YAXXZ
// size      23 bytes
// spans     0x0044CC80-0x0044CC97
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_factionart() {
    VectorDtorIterator(FactionArtGlobal, 0x65C, 8, FactionArtElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_CLOSE_X_SPRITES.
ORIGINAL: 0x0044CCD0
// name      ??__Fg_IFACE_CLOSE_X_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044CCD0-0x0044CCE4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_close_x_sprites() {
    VectorDtorIterator(g_IFACE_CLOSE_X_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES1.
ORIGINAL: 0x0044CD20
// name      ??__Fg_IFACE_BOX_SPRITES1@@YAXXZ
// size      20 bytes
// spans     0x0044CD20-0x0044CD34
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites1() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES1, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES2.
ORIGINAL: 0x0044CD70
// name      ??__Fg_IFACE_BOX_SPRITES2@@YAXXZ
// size      20 bytes
// spans     0x0044CD70-0x0044CD84
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites2() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES2, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES3.
ORIGINAL: 0x0044CDC0
// name      ??__Fg_IFACE_BOX_SPRITES3@@YAXXZ
// size      20 bytes
// spans     0x0044CDC0-0x0044CDD4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites3() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES3, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES4.
ORIGINAL: 0x0044CE10
// name      ??__Fg_IFACE_BOX_SPRITES4@@YAXXZ
// size      20 bytes
// spans     0x0044CE10-0x0044CE24
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites4() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES4, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES5.
ORIGINAL: 0x0044CE60
// name      ??__Fg_IFACE_BOX_SPRITES5@@YAXXZ
// size      20 bytes
// spans     0x0044CE60-0x0044CE74
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites5() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES5, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES6.
ORIGINAL: 0x0044CEB0
// name      ??__Fg_IFACE_BOX_SPRITES6@@YAXXZ
// size      20 bytes
// spans     0x0044CEB0-0x0044CEC4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites6() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES6, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES7.
ORIGINAL: 0x0044CF00
// name      ??__Fg_IFACE_BOX_SPRITES7@@YAXXZ
// size      20 bytes
// spans     0x0044CF00-0x0044CF14
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites7() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES7, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITES8.
ORIGINAL: 0x0044CF50
// name      ??__Fg_IFACE_BOX_SPRITES8@@YAXXZ
// size      20 bytes
// spans     0x0044CF50-0x0044CF64
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprites8() {
    VectorDtorIterator(g_IFACE_BOX_SPRITES8, 0x2C, 51, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_BOX_SPRITE_BUFFERS.
ORIGINAL: 0x0044CFA0
// name      ??__Fg_IFACE_BOX_SPRITE_BUFFERS@@YAXXZ
// size      23 bytes
// spans     0x0044CFA0-0x0044CFB7
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_box_sprite_buffers() {
    VectorDtorIterator(g_IFACE_BOX_SPRITE_BUFFERS, 0x588, 51, BufferElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE.
ORIGINAL: 0x0044CFE0
// name      ??__Fg_IFACE_STD_POPUPS_TOP_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044CFE0-0x0044CFEA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_left_sprite() {
    g_IFACE_STD_POPUPS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE.
ORIGINAL: 0x0044D010
// name      ??__Fg_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D010-0x0044D01A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_right_sprite() {
    g_IFACE_STD_POPUPS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE.
ORIGINAL: 0x0044D040
// name      ??__Fg_IFACE_STD_POPUPS_BOT_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D040-0x0044D04A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_left_sprite() {
    g_IFACE_STD_POPUPS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE.
ORIGINAL: 0x0044D070
// name      ??__Fg_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D070-0x0044D07A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_right_sprite() {
    g_IFACE_STD_POPUPS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_TOP_MID_SPRITE.
ORIGINAL: 0x0044D0A0
// name      ??__Fg_IFACE_STD_POPUPS_TOP_MID_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D0A0-0x0044D0AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_top_mid_sprite() {
    g_IFACE_STD_POPUPS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_BOT_MID_SPRITE.
ORIGINAL: 0x0044D0D0
// name      ??__Fg_IFACE_STD_POPUPS_BOT_MID_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D0D0-0x0044D0DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_bot_mid_sprite() {
    g_IFACE_STD_POPUPS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_LEFT_SPRITE.
ORIGINAL: 0x0044D100
// name      ??__Fg_IFACE_STD_POPUPS_MID_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D100-0x0044D10A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_left_sprite() {
    g_IFACE_STD_POPUPS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE.
ORIGINAL: 0x0044D130
// name      ??__Fg_IFACE_STD_POPUPS_MID_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044D130-0x0044D13A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_mid_right_sprite() {
    g_IFACE_STD_POPUPS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_STD_POPUPS_MIDDLE_BUFFER.
ORIGINAL: 0x0044D160
// name      ??__Fg_IFACE_STD_POPUPS_MIDDLE_BUFFER@@YAXXZ
// size      10 bytes
// spans     0x0044D160-0x0044D16A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_std_popups_middle_buffer() {
    g_IFACE_STD_POPUPS_MIDDLE_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR1.
ORIGINAL: 0x0044D190
// name      ??__Fg_UNUSED_CAVIARDATA_VAR1@@YAXXZ
// size      10 bytes
// spans     0x0044D190-0x0044D19A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var1() {
    g_UNUSED_CAVIARDATA_VAR1->close();
}

/*
Purpose: Atexit teardown thunk for g_SSF_CAVIARDATA.
ORIGINAL: 0x0044D1C0
// name      ??__Fg_SSF_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D1C0-0x0044D1CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssf_caviardata() {
    g_SSF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SDP_CAVIARDATA.
ORIGINAL: 0x0044D1F0
// name      ??__Fg_SDP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D1F0-0x0044D1FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sdp_caviardata() {
    g_SDP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAS_CAVIARDATA.
ORIGINAL: 0x0044D220
// name      ??__Fg_SAS_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D220-0x0044D22A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sas_caviardata() {
    g_SAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCD_CAVIARDATA.
ORIGINAL: 0x0044D250
// name      ??__Fg_SCD_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D250-0x0044D25A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scd_caviardata() {
    g_SCD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SCJ_CAVIARDATA.
ORIGINAL: 0x0044D280
// name      ??__Fg_SCJ_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D280-0x0044D28A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scj_caviardata() {
    g_SCJ_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SAGS_CAVIARDATA.
ORIGINAL: 0x0044D2B0
// name      ??__Fg_SAGS_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D2B0-0x0044D2BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sags_caviardata() {
    g_SAGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFT_CAVIARDATA.
ORIGINAL: 0x0044D2E0
// name      ??__Fg_SFT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D2E0-0x0044D2EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sft_caviardata() {
    g_SFT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA1.
ORIGINAL: 0x0044D310
// name      ??__Fg_VHR_CAVIARDATA1@@YAXXZ
// size      10 bytes
// spans     0x0044D310-0x0044D31A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata1() {
    g_VHR_CAVIARDATA1->close();
}

/*
Purpose: Atexit teardown thunk for g_SHT_CAVIARDATA.
ORIGINAL: 0x0044D340
// name      ??__Fg_SHT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D340-0x0044D34A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sht_caviardata() {
    g_SHT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SRB_CAVIARDATA.
ORIGINAL: 0x0044D370
// name      ??__Fg_SRB_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D370-0x0044D37A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_srb_caviardata() {
    g_SRB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASAS_CAVIARDATA.
ORIGINAL: 0x0044D3A0
// name      ??__Fg_ASAS_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D3A0-0x0044D3AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asas_caviardata() {
    g_ASAS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESLASER_CAVIARDATA.
ORIGINAL: 0x0044D3D0
// name      ??__Fg_RESLASER_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D3D0-0x0044D3DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reslaser_caviardata() {
    g_RESLASER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_RESBOLT_CAVIARDATA.
ORIGINAL: 0x0044D400
// name      ??__Fg_RESBOLT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D400-0x0044D40A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resbolt_caviardata() {
    g_RESBOLT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_FUNLOAD_CAVIARDATA.
ORIGINAL: 0x0044D430
// name      ??__Fg_FUNLOAD_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D430-0x0044D43A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_funload_caviardata() {
    g_FUNLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_TECLOAD_CAVIARDATA.
ORIGINAL: 0x0044D460
// name      ??__Fg_TECLOAD_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D460-0x0044D46A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tecload_caviardata() {
    g_TECLOAD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_DISSWAVE_CAVIARDATA.
ORIGINAL: 0x0044D490
// name      ??__Fg_SP_DISSWAVE_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D490-0x0044D49A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_disswave_caviardata() {
    g_SP_DISSWAVE_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_MARINED_CAVIARDATA.
ORIGINAL: 0x0044D4C0
// name      ??__Fg_SP_MARINED_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D4C0-0x0044D4CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_marined_caviardata() {
    g_SP_MARINED_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_NANOO_CAVIARDATA.
ORIGINAL: 0x0044D4F0
// name      ??__Fg_SP_NANOO_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D4F0-0x0044D4FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_nanoo_caviardata() {
    g_SP_NANOO_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SP_SOPORIFIC_CAVIARDATA.
ORIGINAL: 0x0044D520
// name      ??__Fg_SP_SOPORIFIC_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D520-0x0044D52A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sp_soporific_caviardata() {
    g_SP_SOPORIFIC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA01_CAVIARDATA.
ORIGINAL: 0x0044D550
// name      ??__Fg_AA01_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D550-0x0044D55A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa01_caviardata() {
    g_AA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_ROVER_CAVIARDATA.
ORIGINAL: 0x0044D580
// name      ??__Fg_AA_ROVER_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D580-0x0044D58A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_rover_caviardata() {
    g_AA_ROVER_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_WING_CAVIARDATA.
ORIGINAL: 0x0044D5C0
// name      ??__Fg_AA_WING_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044D5C0-0x0044D5D4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_wing_caviardata() {
    VectorDtorIterator(g_AA_WING_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ACP_CAVIARDATA.
ORIGINAL: 0x0044D610
// name      ??__Fg_ACP_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044D610-0x0044D624
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acp_caviardata() {
    VectorDtorIterator(g_ACP_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AX_CAVIARDATA.
ORIGINAL: 0x0044D650
// name      ??__Fg_AX_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D650-0x0044D65A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ax_caviardata() {
    g_AX_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AA_CAVIARDATA.
ORIGINAL: 0x0044D680
// name      ??__Fg_AA_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D680-0x0044D68A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_aa_caviardata() {
    g_AA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ACOLPOD_CAVIARDATA.
ORIGINAL: 0x0044D6B0
// name      ??__Fg_ACOLPOD_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D6B0-0x0044D6BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_acolpod_caviardata() {
    g_ACOLPOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_AT_CAVIARDATA.
ORIGINAL: 0x0044D6E0
// name      ??__Fg_AT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D6E0-0x0044D6EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_at_caviardata() {
    g_AT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VTA_CAVIARDATA.
ORIGINAL: 0x0044D710
// name      ??__Fg_VTA_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D710-0x0044D71A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vta_caviardata() {
    g_VTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ATP_CAVIARDATA.
ORIGINAL: 0x0044D740
// name      ??__Fg_ATP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D740-0x0044D74A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_atp_caviardata() {
    g_ATP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SSFA_CAVIARDATA.
ORIGINAL: 0x0044D770
// name      ??__Fg_SSFA_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D770-0x0044D77A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ssfa_caviardata() {
    g_SSFA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_SFTA_CAVIARDATA.
ORIGINAL: 0x0044D7A0
// name      ??__Fg_SFTA_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D7A0-0x0044D7AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sfta_caviardata() {
    g_SFTA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW00_CAVIARDATA.
ORIGINAL: 0x0044D7D0
// name      ??__Fg_VW00_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D7D0-0x0044D7DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw00_caviardata() {
    g_VW00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAWL_CAVIARDATA.
ORIGINAL: 0x0044D800
// name      ??__Fg_VIPTAWL_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D800-0x0044D80A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptawl_caviardata() {
    g_VIPTAWL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTASGN_CAVIARDATA.
ORIGINAL: 0x0044D830
// name      ??__Fg_VIPTASGN_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D830-0x0044D83A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptasgn_caviardata() {
    g_VIPTASGN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTAPSI_CAVIARDATA.
ORIGINAL: 0x0044D860
// name      ??__Fg_VIPTAPSI_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D860-0x0044D86A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptapsi_caviardata() {
    g_VIPTAPSI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VIPTR_CAVIARDATA.
ORIGINAL: 0x0044D8A0
// name      ??__Fg_VIPTR_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044D8A0-0x0044D8B4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_viptr_caviardata() {
    VectorDtorIterator(g_VIPTR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VIPTA_CAVIARDATA.
ORIGINAL: 0x0044D8F0
// name      ??__Fg_VIPTA_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044D8F0-0x0044D904
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vipta_caviardata() {
    VectorDtorIterator(g_VIPTA_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_PTMOD_CAVIARDATA.
ORIGINAL: 0x0044D930
// name      ??__Fg_PTMOD_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D930-0x0044D93A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ptmod_caviardata() {
    g_PTMOD_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VB_CAVIARDATA.
ORIGINAL: 0x0044D960
// name      ??__Fg_VB_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D960-0x0044D96A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vb_caviardata() {
    g_VB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VBP_CAVIARDATA.
ORIGINAL: 0x0044D990
// name      ??__Fg_VBP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D990-0x0044D99A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vbp_caviardata() {
    g_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMC_CAVIARDATA.
ORIGINAL: 0x0044D9C0
// name      ??__Fg_VGMC_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D9C0-0x0044D9CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmc_caviardata() {
    g_VGMC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMCP_CAVIARDATA.
ORIGINAL: 0x0044D9F0
// name      ??__Fg_VGMCP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044D9F0-0x0044D9FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmcp_caviardata() {
    g_VGMCP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VLIGHTS_CAVIARDATA.
ORIGINAL: 0x0044DA20
// name      ??__Fg_VLIGHTS_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DA20-0x0044DA2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vlights_caviardata() {
    g_VLIGHTS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPT_CAVIARDATA.
ORIGINAL: 0x0044DA50
// name      ??__Fg_VPT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DA50-0x0044DA5A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpt_caviardata() {
    g_VPT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_A_CAVIARDATA.
ORIGINAL: 0x0044DA80
// name      ??__Fg_A_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DA80-0x0044DA8A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_a_caviardata() {
    g_A_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APWALL_CAVIARDATA.
ORIGINAL: 0x0044DAB0
// name      ??__Fg_APWALL_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DAB0-0x0044DABA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apwall_caviardata() {
    g_APWALL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_ASGEN_CAVIARDATA.
ORIGINAL: 0x0044DAE0
// name      ??__Fg_ASGEN_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DAE0-0x0044DAEA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_asgen_caviardata() {
    g_ASGEN_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_APSID_CAVIARDATA.
ORIGINAL: 0x0044DB10
// name      ??__Fg_APSID_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DB10-0x0044DB1A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_apsid_caviardata() {
    g_APSID_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VA01_CAVIARDATA.
ORIGINAL: 0x0044DB40
// name      ??__Fg_VA01_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DB40-0x0044DB4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_va01_caviardata() {
    g_VA01_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VR_CAVIARDATA.
ORIGINAL: 0x0044DB80
// name      ??__Fg_VR_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044DB80-0x0044DB94
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vr_caviardata() {
    VectorDtorIterator(g_VR_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHR_CAVIARDATA2.
ORIGINAL: 0x0044DBC0
// name      ??__Fg_VHR_CAVIARDATA2@@YAXXZ
// size      10 bytes
// spans     0x0044DBC0-0x0044DBCA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhr_caviardata2() {
    g_VHR_CAVIARDATA2->close();
}

/*
Purpose: Atexit teardown thunk for g_VRC_CAVIARDATA.
ORIGINAL: 0x0044DC00
// name      ??__Fg_VRC_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044DC00-0x0044DC14
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vrc_caviardata() {
    VectorDtorIterator(g_VRC_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VI_CAVIARDATA.
ORIGINAL: 0x0044DC40
// name      ??__Fg_VI_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DC40-0x0044DC4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vi_caviardata() {
    g_VI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMT_CAVIARDATA.
ORIGINAL: 0x0044DC70
// name      ??__Fg_VGMT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DC70-0x0044DC7A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmt_caviardata() {
    g_VGMT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGMTP_CAVIARDATA.
ORIGINAL: 0x0044DCA0
// name      ??__Fg_VGMTP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DCA0-0x0044DCAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgmtp_caviardata() {
    g_VGMTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR2.
ORIGINAL: 0x0044DCD0
// name      ??__Fg_UNUSED_CAVIARDATA_VAR2@@YAXXZ
// size      10 bytes
// spans     0x0044DCD0-0x0044DCDA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var2() {
    g_UNUSED_CAVIARDATA_VAR2->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNTU_CAVIARDATA.
ORIGINAL: 0x0044DD00
// name      ??__Fg_VWNTU_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DD00-0x0044DD0A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntu_caviardata() {
    g_VWNTU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VT_CAVIARDATA.
ORIGINAL: 0x0044DD30
// name      ??__Fg_VT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DD30-0x0044DD3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vt_caviardata() {
    g_VT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROP_CAVIARDATA.
ORIGINAL: 0x0044DD60
// name      ??__Fg_DROP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DD60-0x0044DD6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_drop_caviardata() {
    g_DROP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_DROPLET_CAVIARDATA.
ORIGINAL: 0x0044DD90
// name      ??__Fg_DROPLET_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DD90-0x0044DD9A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_droplet_caviardata() {
    g_DROPLET_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCL_CAVIARDATA.
ORIGINAL: 0x0044DDC0
// name      ??__Fg_VCL_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DDC0-0x0044DDCA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcl_caviardata() {
    g_VCL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR3.
ORIGINAL: 0x0044DE00
// name      ??__Fg_UNUSED_CAVIARDATA_VAR3@@YAXXZ
// size      20 bytes
// spans     0x0044DE00-0x0044DE14
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var3() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR3, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCLT00_CAVIARDATA.
ORIGINAL: 0x0044DE40
// name      ??__Fg_VCLT00_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DE40-0x0044DE4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vclt00_caviardata() {
    g_VCLT00_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHT_VBP_CAVIARDATA.
ORIGINAL: 0x0044DE70
// name      ??__Fg_VHT_VBP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DE70-0x0044DE7A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vht_vbp_caviardata() {
    g_VHT_VBP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTP_CAVIARDATA.
ORIGINAL: 0x0044DEA0
// name      ??__Fg_VHTP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DEA0-0x0044DEAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhtp_caviardata() {
    g_VHTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTA0_CAVIARDATA.
ORIGINAL: 0x0044DEE0
// name      ??__Fg_VHTA0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044DEE0-0x0044DEF4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhta0_caviardata() {
    VectorDtorIterator(g_VHTA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VHTTP_CAVIARDATA.
ORIGINAL: 0x0044DF20
// name      ??__Fg_VHTTP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DF20-0x0044DF2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttp_caviardata() {
    g_VHTTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VHTTPA0_CAVIARDATA.
ORIGINAL: 0x0044DF60
// name      ??__Fg_VHTTPA0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044DF60-0x0044DF74
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vhttpa0_caviardata() {
    VectorDtorIterator(g_VHTTPA0_CAVIARDATA, 0xC, 3, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSP_CAVIARDATA.
ORIGINAL: 0x0044DFA0
// name      ??__Fg_VSP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044DFA0-0x0044DFAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsp_caviardata() {
    g_VSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPA0_CAVIARDATA.
ORIGINAL: 0x0044DFE0
// name      ??__Fg_VSPA0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044DFE0-0x0044DFF4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vspa0_caviardata() {
    VectorDtorIterator(g_VSPA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VSPTF_CAVIARDATA.
ORIGINAL: 0x0044E020
// name      ??__Fg_VSPTF_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E020-0x0044E02A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptf_caviardata() {
    g_VSPTF_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VSPTB_CAVIARDATA.
ORIGINAL: 0x0044E050
// name      ??__Fg_VSPTB_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E050-0x0044E05A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vsptb_caviardata() {
    g_VSPTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VFL_CAVIARDATA.
ORIGINAL: 0x0044E080
// name      ??__Fg_VFL_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E080-0x0044E08A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vfl_caviardata() {
    g_VFL_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGS_CAVIARDATA.
ORIGINAL: 0x0044E0B0
// name      ??__Fg_VGS_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E0B0-0x0044E0BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgs_caviardata() {
    g_VGS_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VGSP_CAVIARDATA.
ORIGINAL: 0x0044E0E0
// name      ??__Fg_VGSP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E0E0-0x0044E0EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vgsp_caviardata() {
    g_VGSP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJTP_CAVIARDATA.
ORIGINAL: 0x0044E110
// name      ??__Fg_VJTP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E110-0x0044E11A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjtp_caviardata() {
    g_VJTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VJT0_CAVIARDATA.
ORIGINAL: 0x0044E150
// name      ??__Fg_VJT0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044E150-0x0044E164
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vjt0_caviardata() {
    VectorDtorIterator(g_VJT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCU_CAVIARDATA.
ORIGINAL: 0x0044E190
// name      ??__Fg_VCU_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E190-0x0044E19A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcu_caviardata() {
    g_VCU_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUP_CAVIARDATA.
ORIGINAL: 0x0044E1C0
// name      ??__Fg_VCUP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E1C0-0x0044E1CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcup_caviardata() {
    g_VCUP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCUA0_CAVIARDATA.
ORIGINAL: 0x0044E200
// name      ??__Fg_VCUA0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044E200-0x0044E214
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcua0_caviardata() {
    VectorDtorIterator(g_VCUA0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VCUW_CAVIARDATA.
ORIGINAL: 0x0044E240
// name      ??__Fg_VCUW_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E240-0x0044E24A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vcuw_caviardata() {
    g_VCUW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT_CAVIARDATA.
ORIGINAL: 0x0044E270
// name      ??__Fg_VCT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E270-0x0044E27A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct_caviardata() {
    g_VCT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTP_CAVIARDATA.
ORIGINAL: 0x0044E2A0
// name      ??__Fg_VCTP_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E2A0-0x0044E2AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctp_caviardata() {
    g_VCTP_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCTB_CAVIARDATA.
ORIGINAL: 0x0044E2D0
// name      ??__Fg_VCTB_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E2D0-0x0044E2DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vctb_caviardata() {
    g_VCTB_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VCT0_CAVIARDATA.
ORIGINAL: 0x0044E310
// name      ??__Fg_VCT0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044E310-0x0044E324
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vct0_caviardata() {
    VectorDtorIterator(g_VCT0_CAVIARDATA, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VWNTT_CAVIARDATA.
ORIGINAL: 0x0044E350
// name      ??__Fg_VWNTT_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E350-0x0044E35A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwntt_caviardata() {
    g_VWNTT_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNST_CAVIARDATA.
ORIGINAL: 0x0044E380
// name      ??__Fg_VWNST_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E380-0x0044E38A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnst_caviardata() {
    g_VWNST_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VWNAA_CAVIARDATA.
ORIGINAL: 0x0044E3B0
// name      ??__Fg_VWNAA_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E3B0-0x0044E3BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vwnaa_caviardata() {
    g_VWNAA_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VW_CAVIARDATA.
ORIGINAL: 0x0044E3F0
// name      ??__Fg_VW_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044E3F0-0x0044E404
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vw_caviardata() {
    VectorDtorIterator(g_VW_CAVIARDATA, 0xC, 16, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VM_CAVIARDATA.
ORIGINAL: 0x0044E430
// name      ??__Fg_VM_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E430-0x0044E43A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm_caviardata() {
    g_VM_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VM13_CAVIARDATA.
ORIGINAL: 0x0044E460
// name      ??__Fg_VM13_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E460-0x0044E46A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vm13_caviardata() {
    g_VM13_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_VPBR0_CAVIARDATA.
ORIGINAL: 0x0044E4A0
// name      ??__Fg_VPBR0_CAVIARDATA@@YAXXZ
// size      20 bytes
// spans     0x0044E4A0-0x0044E4B4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vpbr0_caviardata() {
    VectorDtorIterator(g_VPBR0_CAVIARDATA, 0xC, 4, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR4.
ORIGINAL: 0x0044E4F0
// name      ??__Fg_UNUSED_CAVIARDATA_VAR4@@YAXXZ
// size      20 bytes
// spans     0x0044E4F0-0x0044E504
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var4() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR4, 0xC, 2, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NW_CAVIARDATA.
ORIGINAL: 0x0044E530
// name      ??__Fg_NW_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E530-0x0044E53A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nw_caviardata() {
    g_NW_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NI_CAVIARDATA.
ORIGINAL: 0x0044E560
// name      ??__Fg_NI_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E560-0x0044E56A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ni_caviardata() {
    g_NI_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_NLC_CAVIARDATA.
ORIGINAL: 0x0044E590
// name      ??__Fg_NLC_CAVIARDATA@@YAXXZ
// size      10 bytes
// spans     0x0044E590-0x0044E59A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_nlc_caviardata() {
    g_NLC_CAVIARDATA->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_CAVIARDATA_VAR5.
ORIGINAL: 0x0044E5D0
// name      ??__Fg_UNUSED_CAVIARDATA_VAR5@@YAXXZ
// size      20 bytes
// spans     0x0044E5D0-0x0044E5E4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_caviardata_var5() {
    VectorDtorIterator(g_UNUSED_CAVIARDATA_VAR5, 0xC, 5, CaviarDataElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RADIUS1_TEXTURE.
ORIGINAL: 0x0044E610
// name      ??__Fg_RADIUS1_TEXTURE@@YAXXZ
// size      10 bytes
// spans     0x0044E610-0x0044E61A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius1_texture() {
    g_RADIUS1_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_RADIUS2_TEXTURE.
ORIGINAL: 0x0044E640
// name      ??__Fg_RADIUS2_TEXTURE@@YAXXZ
// size      10 bytes
// spans     0x0044E640-0x0044E64A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_radius2_texture() {
    g_RADIUS2_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_ROCKY_TEXTURES.
ORIGINAL: 0x0044E680
// name      ??__Fg_ROCKY_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E680-0x0044E694
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rocky_textures() {
    VectorDtorIterator(g_ROCKY_TEXTURES, 0x70, 4, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_OCEAN_TEXTURES.
ORIGINAL: 0x0044E6D0
// name      ??__Fg_OCEAN_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E6D0-0x0044E6E4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ocean_textures() {
    VectorDtorIterator(g_OCEAN_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAT_ARID_LAND_TEXTURE.
ORIGINAL: 0x0044E710
// name      ??__Fg_FLAT_ARID_LAND_TEXTURE@@YAXXZ
// size      10 bytes
// spans     0x0044E710-0x0044E71A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flat_arid_land_texture() {
    g_FLAT_ARID_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_MOIST_LAND_TEXTURES.
ORIGINAL: 0x0044E750
// name      ??__Fg_MOIST_LAND_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E750-0x0044E764
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_moist_land_textures() {
    VectorDtorIterator(g_MOIST_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINY_LAND_TEXTURES.
ORIGINAL: 0x0044E7A0
// name      ??__Fg_RAINY_LAND_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E7A0-0x0044E7B4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainy_land_textures() {
    VectorDtorIterator(g_RAINY_LAND_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_JUNGLE_LAND_TEXTURES.
ORIGINAL: 0x0044E7F0
// name      ??__Fg_JUNGLE_LAND_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E7F0-0x0044E804
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jungle_land_textures() {
    VectorDtorIterator(g_JUNGLE_LAND_TEXTURES, 0x70, 15, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_DUNE_LAND_TEXTURE.
ORIGINAL: 0x0044E830
// name      ??__Fg_DUNE_LAND_TEXTURE@@YAXXZ
// size      10 bytes
// spans     0x0044E830-0x0044E83A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_dune_land_texture() {
    g_DUNE_LAND_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_SUNNY_MESA_TEXTURES.
ORIGINAL: 0x0044E870
// name      ??__Fg_SUNNY_MESA_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E870-0x0044E884
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_sunny_mesa_textures() {
    VectorDtorIterator(g_SUNNY_MESA_TEXTURES, 0x70, 8, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURES.
ORIGINAL: 0x0044E8C0
// name      ??__Fg_RAINFALL_SINGLE_TILE_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E8C0-0x0044E8D4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_textures() {
    VectorDtorIterator(g_RAINFALL_SINGLE_TILE_TEXTURES, 0x70, 2, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ROAD_TEXTURES.
ORIGINAL: 0x0044E910
// name      ??__Fg_ROAD_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E910-0x0044E924
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_road_textures() {
    VectorDtorIterator(g_ROAD_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MAGTUBE_TEXTURES.
ORIGINAL: 0x0044E960
// name      ??__Fg_MAGTUBE_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E960-0x0044E974
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_magtube_textures() {
    VectorDtorIterator(g_MAGTUBE_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RIVER_TEXTURES.
ORIGINAL: 0x0044E9B0
// name      ??__Fg_RIVER_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044E9B0-0x0044E9C4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_river_textures() {
    VectorDtorIterator(g_RIVER_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MOUNT_PLANET_TEXTURES.
ORIGINAL: 0x0044EA00
// name      ??__Fg_MOUNT_PLANET_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044EA00-0x0044EA14
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_mount_planet_textures() {
    VectorDtorIterator(g_MOUNT_PLANET_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GARLAND_CRATER_TEXTURES.
ORIGINAL: 0x0044EA50
// name      ??__Fg_GARLAND_CRATER_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044EA50-0x0044EA64
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_garland_crater_textures() {
    VectorDtorIterator(g_GARLAND_CRATER_TEXTURES, 0x70, 3, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FUNGUS_TEXTURES.
ORIGINAL: 0x0044EAA0
// name      ??__Fg_FUNGUS_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044EAA0-0x0044EAB4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fungus_textures() {
    VectorDtorIterator(g_FUNGUS_TEXTURES, 0x70, 30, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FARM_TEXTURES.
ORIGINAL: 0x0044EAF0
// name      ??__Fg_FARM_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044EAF0-0x0044EB04
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_farm_textures() {
    VectorDtorIterator(g_FARM_TEXTURES, 0x70, 9, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOREST_TEXTURES.
ORIGINAL: 0x0044EB40
// name      ??__Fg_FOREST_TEXTURES@@YAXXZ
// size      20 bytes
// spans     0x0044EB40-0x0044EB54
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_forest_textures() {
    VectorDtorIterator(g_FOREST_TEXTURES, 0x70, 16, TextureElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_SINGLE_TILE_TEXTURE.
ORIGINAL: 0x0044EB80
// name      ??__Fg_RAINFALL_SINGLE_TILE_TEXTURE@@YAXXZ
// size      10 bytes
// spans     0x0044EB80-0x0044EB8A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_single_tile_texture() {
    g_RAINFALL_SINGLE_TILE_TEXTURE->~Texture();
}

/*
Purpose: Atexit teardown thunk for g_TER1_WHITE_ORG_YEL_TILE_SPRITES.
ORIGINAL: 0x0044EBC0
// name      ??__Fg_TER1_WHITE_ORG_YEL_TILE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EBC0-0x0044EBD4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_white_org_yel_tile_sprites() {
    VectorDtorIterator(g_TER1_WHITE_ORG_YEL_TILE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOTTOM_LEFT_TILE_SPRITES.
ORIGINAL: 0x0044EC10
// name      ??__Fg_TER1_BOTTOM_LEFT_TILE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EC10-0x0044EC24
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bottom_left_tile_sprites() {
    VectorDtorIterator(g_TER1_BOTTOM_LEFT_TILE_SPRITES, 0x2C, 9, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES2.
ORIGINAL: 0x0044EC60
// name      ??__Fg_TER1_UNUSED_SPRITES2@@YAXXZ
// size      20 bytes
// spans     0x0044EC60-0x0044EC74
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites2() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES2, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINE_SPRITE.
ORIGINAL: 0x0044ECA0
// name      ??__Fg_TER1_MINE_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ECA0-0x0044ECAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mine_sprite() {
    g_TER1_MINE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOLAR_COLLECTOR_SPRITE.
ORIGINAL: 0x0044ECD0
// name      ??__Fg_TER1_SOLAR_COLLECTOR_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ECD0-0x0044ECDA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_solar_collector_sprite() {
    g_TER1_SOLAR_COLLECTOR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TIDAL_HARNESS_SPRITE.
ORIGINAL: 0x0044ED00
// name      ??__Fg_TER1_TIDAL_HARNESS_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ED00-0x0044ED0A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tidal_harness_sprite() {
    g_TER1_TIDAL_HARNESS_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MINING_PLATFORM_SPRITE.
ORIGINAL: 0x0044ED30
// name      ??__Fg_TER1_MINING_PLATFORM_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ED30-0x0044ED3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_mining_platform_sprite() {
    g_TER1_MINING_PLATFORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_TUT_BLANK_SPRITE.
ORIGINAL: 0x0044ED60
// name      ??__Fg_TER1_TUT_BLANK_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ED60-0x0044ED6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_tut_blank_sprite() {
    g_TER1_TUT_BLANK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_KELP_FARM_SPRITE.
ORIGINAL: 0x0044ED90
// name      ??__Fg_TER1_KELP_FARM_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044ED90-0x0044ED9A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_kelp_farm_sprite() {
    g_TER1_KELP_FARM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_CONDENSER_SPRITE.
ORIGINAL: 0x0044EDC0
// name      ??__Fg_TER1_CONDENSER_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044EDC0-0x0044EDCA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_condenser_sprite() {
    g_TER1_CONDENSER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_ECHELON_MIRROR_SPRITE.
ORIGINAL: 0x0044EDF0
// name      ??__Fg_TER1_ECHELON_MIRROR_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044EDF0-0x0044EDFA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_echelon_mirror_sprite() {
    g_TER1_ECHELON_MIRROR_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_SPRITE.
ORIGINAL: 0x0044EE20
// name      ??__Fg_TER1_BOREHOLE_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044EE20-0x0044EE2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_sprite() {
    g_TER1_BOREHOLE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BOREHOLE_CLUSTER_SPRITE.
ORIGINAL: 0x0044EE50
// name      ??__Fg_TER1_BOREHOLE_CLUSTER_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044EE50-0x0044EE5A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_borehole_cluster_sprite() {
    g_TER1_BOREHOLE_CLUSTER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_MANIFOLD_NEXUS_SPRITES.
ORIGINAL: 0x0044EE90
// name      ??__Fg_TER1_MANIFOLD_NEXUS_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EE90-0x0044EEA4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_manifold_nexus_sprites() {
    VectorDtorIterator(g_TER1_MANIFOLD_NEXUS_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_SPRITES.
ORIGINAL: 0x0044EEE0
// name      ??__Fg_TER1WRECK_UNITY_WRECKAGE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EEE0-0x0044EEF4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_SPRITES, 0x2C, 15, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES.
ORIGINAL: 0x0044EF30
// name      ??__Fg_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EF30-0x0044EF44
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1wreck_unity_wreckage_alt_sprites() {
    VectorDtorIterator(g_TER1WRECK_UNITY_WRECKAGE_ALT_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FOSSIL_FIELD_RIDGE_SPRITES.
ORIGINAL: 0x0044EF80
// name      ??__Fg_FOSSIL_FIELD_RIDGE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044EF80-0x0044EF94
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fossil_field_ridge_sprites() {
    VectorDtorIterator(g_FOSSIL_FIELD_RIDGE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNUSED_SPRITES1.
ORIGINAL: 0x0044EFD0
// name      ??__Fg_TER1_UNUSED_SPRITES1@@YAXXZ
// size      20 bytes
// spans     0x0044EFD0-0x0044EFE4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unused_sprites1() {
    VectorDtorIterator(g_TER1_UNUSED_SPRITES1, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_FARM_SPRITES.
ORIGINAL: 0x0044F020
// name      ??__Fg_TER1_FARM_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F020-0x0044F034
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_farm_sprites() {
    VectorDtorIterator(g_TER1_FARM_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SOIL_ENRICHER_SPRITES.
ORIGINAL: 0x0044F070
// name      ??__Fg_TER1_SOIL_ENRICHER_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F070-0x0044F084
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_soil_enricher_sprites() {
    VectorDtorIterator(g_TER1_SOIL_ENRICHER_SPRITES, 0x2C, 5, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_SEA_LAND_RESOURCE_SPRITES.
ORIGINAL: 0x0044F0C0
// name      ??__Fg_TER1_SEA_LAND_RESOURCE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F0C0-0x0044F0D4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sea_land_resource_sprites() {
    VectorDtorIterator(g_TER1_SEA_LAND_RESOURCE_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_LANDMARK_RESOURCE_SPRITES.
ORIGINAL: 0x0044F110
// name      ??__Fg_TER1_LANDMARK_RESOURCE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F110-0x0044F124
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_landmark_resource_sprites() {
    VectorDtorIterator(g_TER1_LANDMARK_RESOURCE_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_GLOW_SPRITES.
ORIGINAL: 0x0044F160
// name      ??__Fg_GLOW_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F160-0x0044F174
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_glow_sprites() {
    VectorDtorIterator(g_GLOW_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_UNITY_POD_SPRITES.
ORIGINAL: 0x0044F1B0
// name      ??__Fg_TER1_UNITY_POD_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F1B0-0x0044F1C4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_unity_pod_sprites() {
    VectorDtorIterator(g_TER1_UNITY_POD_SPRITES, 0x2C, 6, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_TER1_MONOLITH_SPRITE.
ORIGINAL: 0x0044F1F0
// name      ??__Fg_TER1_MONOLITH_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F1F0-0x0044F1FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_monolith_sprite() {
    g_TER1_MONOLITH_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_BUNKER_SPRITE.
ORIGINAL: 0x0044F220
// name      ??__Fg_TER1_BUNKER_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F220-0x0044F22A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_bunker_sprite() {
    g_TER1_BUNKER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_AIRBASE_SPRITE.
ORIGINAL: 0x0044F250
// name      ??__Fg_TER1_AIRBASE_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F250-0x0044F25A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_airbase_sprite() {
    g_TER1_AIRBASE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TER1_SENSOR_ARRAY_SPRITE.
ORIGINAL: 0x0044F280
// name      ??__Fg_TER1_SENSOR_ARRAY_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F280-0x0044F28A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_ter1_sensor_array_sprite() {
    g_TER1_SENSOR_ARRAY_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_RAINFALL_DOUBLE_TILE_SPRITES.
ORIGINAL: 0x0044F2C0
// name      ??__Fg_RAINFALL_DOUBLE_TILE_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F2C0-0x0044F2D4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_rainfall_double_tile_sprites() {
    VectorDtorIterator(g_RAINFALL_DOUBLE_TILE_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_VEH_SPRITES.
ORIGINAL: 0x0044F310
// name      ??__Fg_VEH_SPRITES@@YAXXZ
// size      23 bytes
// spans     0x0044F310-0x0044F327
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_veh_sprites() {
    VectorDtorIterator(g_VEH_SPRITES, 0x2C, 152, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FLAGS_VEH_SPRITES.
ORIGINAL: 0x0044F360
// name      ??__Fg_FLAGS_VEH_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F360-0x0044F374
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_flags_veh_sprites() {
    VectorDtorIterator(g_FLAGS_VEH_SPRITES, 0x2C, 112, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICONS_GENERAL_SPRITES.
ORIGINAL: 0x0044F3B0
// name      ??__Fg_ICONS_GENERAL_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F3B0-0x0044F3C4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icons_general_sprites() {
    VectorDtorIterator(g_ICONS_GENERAL_SPRITES, 0x2C, 16, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RESOURCE_ICON_SPRITES.
ORIGINAL: 0x0044F400
// name      ??__Fg_RESOURCE_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F400-0x0044F414
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_resource_icon_sprites() {
    VectorDtorIterator(g_RESOURCE_ICON_SPRITES, 0x2C, 32, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_LG_CURSOR_SPRITES.
ORIGINAL: 0x0044F450
// name      ??__Fg_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F450-0x0044F464
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_LG_CURSOR_SPRITES.
ORIGINAL: 0x0044F4A0
// name      ??__Fg_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F4A0-0x0044F4B4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CITIZEN_SM_CURSOR_SPRITES.
ORIGINAL: 0x0044F4F0
// name      ??__Fg_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F4F0-0x0044F504
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SPECIALIST_SM_CURSOR_SPRITES.
ORIGINAL: 0x0044F540
// name      ??__Fg_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F540-0x0044F554
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_LG_CURSOR_SPRITES.
ORIGINAL: 0x0044F590
// name      ??__Fg_AL_CITIZEN_LG_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F590-0x0044F5A4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_LG_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_LG_CURSOR_SPRITES.
ORIGINAL: 0x0044F5E0
// name      ??__Fg_AL_SPECIALIST_LG_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F5E0-0x0044F5F4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_lg_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_LG_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_CITIZEN_SM_CURSOR_SPRITES.
ORIGINAL: 0x0044F630
// name      ??__Fg_AL_CITIZEN_SM_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F630-0x0044F644
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_citizen_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_CITIZEN_SM_CURSOR_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_AL_SPECIALIST_SM_CURSOR_SPRITES.
ORIGINAL: 0x0044F680
// name      ??__Fg_AL_SPECIALIST_SM_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F680-0x0044F694
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_al_specialist_sm_cursor_sprites() {
    VectorDtorIterator(g_AL_SPECIALIST_SM_CURSOR_SPRITES, 0x2C, 7, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_ALIEN_HEAD_ICON_SPRITE.
ORIGINAL: 0x0044F6C0
// name      ??__Fg_RED_ALIEN_HEAD_ICON_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F6C0-0x0044F6CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_alien_head_icon_sprite() {
    g_RED_ALIEN_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_SILVER_MENU_ICON_SPRITES.
ORIGINAL: 0x0044F700
// name      ??__Fg_SILVER_MENU_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F700-0x0044F714
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_menu_icon_sprites() {
    VectorDtorIterator(g_SILVER_MENU_ICON_SPRITES, 0x2C, 4, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SILVER_CHECKBOX_ICON_SPRITES.
ORIGINAL: 0x0044F750
// name      ??__Fg_SILVER_CHECKBOX_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F750-0x0044F764
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_silver_checkbox_icon_sprites() {
    VectorDtorIterator(g_SILVER_CHECKBOX_ICON_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_RED_MALE_HEAD_ICON_SPRITE.
ORIGINAL: 0x0044F790
// name      ??__Fg_RED_MALE_HEAD_ICON_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F790-0x0044F79A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_red_male_head_icon_sprite() {
    g_RED_MALE_HEAD_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_NULL_RESOURCE_ICON_SPRITE.
ORIGINAL: 0x0044F7C0
// name      ??__Fg_NULL_RESOURCE_ICON_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F7C0-0x0044F7CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_null_resource_icon_sprite() {
    g_NULL_RESOURCE_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_PEACE_SIGN_SPRITES.
ORIGINAL: 0x0044F800
// name      ??__Fg_PEACE_SIGN_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F800-0x0044F814
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_peace_sign_sprites() {
    VectorDtorIterator(g_PEACE_SIGN_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_ICON_TILE_SQUARE_SPRITE.
ORIGINAL: 0x0044F840
// name      ??__Fg_ICON_TILE_SQUARE_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F840-0x0044F84A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_icon_tile_square_sprite() {
    g_ICON_TILE_SQUARE_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_XI_BOOM_VEH_SPRITES.
ORIGINAL: 0x0044F880
// name      ??__Fg_XI_BOOM_VEH_SPRITES@@YAXXZ
// size      23 bytes
// spans     0x0044F880-0x0044F897
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xi_boom_veh_sprites() {
    VectorDtorIterator(g_XI_BOOM_VEH_SPRITES, 0x2C, 144, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_XF_BOOM_VEH_SPRITES.
ORIGINAL: 0x0044F8D0
// name      ??__Fg_XF_BOOM_VEH_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044F8D0-0x0044F8E4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_xf_boom_veh_sprites() {
    VectorDtorIterator(g_XF_BOOM_VEH_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_MIND_WORM_SPRITE.
ORIGINAL: 0x0044F910
// name      ??__Fg_BATTLE_MIND_WORM_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F910-0x0044F91A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_mind_worm_sprite() {
    g_BATTLE_MIND_WORM_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_ISLE_DEEP_SPRITE.
ORIGINAL: 0x0044F940
// name      ??__Fg_BATTLE_ISLE_DEEP_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F940-0x0044F94A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_isle_deep_sprite() {
    g_BATTLE_ISLE_DEEP_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_LOCUSTS_CHIRON_SPRITE.
ORIGINAL: 0x0044F970
// name      ??__Fg_BATTLE_LOCUSTS_CHIRON_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F970-0x0044F97A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_locusts_chiron_sprite() {
    g_BATTLE_LOCUSTS_CHIRON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_FUNGAL_TOWER_SPRITE.
ORIGINAL: 0x0044F9A0
// name      ??__Fg_BATTLE_FUNGAL_TOWER_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F9A0-0x0044F9AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_fungal_tower_sprite() {
    g_BATTLE_FUNGAL_TOWER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SPORE_LAUNCHER_SPRITE.
ORIGINAL: 0x0044F9D0
// name      ??__Fg_BATTLE_SPORE_LAUNCHER_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044F9D0-0x0044F9DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_spore_launcher_sprite() {
    g_BATTLE_SPORE_LAUNCHER_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_BATTLE_SEALURK_SPRITE.
ORIGINAL: 0x0044FA00
// name      ??__Fg_BATTLE_SEALURK_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FA00-0x0044FA0A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_battle_sealurk_sprite() {
    g_BATTLE_SEALURK_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_TECH_ICON_SPRITES.
ORIGINAL: 0x0044FA40
// name      ??__Fg_TECH_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FA40-0x0044FA54
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tech_icon_sprites() {
    VectorDtorIterator(g_TECH_ICON_SPRITES, 0x2C, 89, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_FACILITY_ICON_SPRITES.
ORIGINAL: 0x0044FA90
// name      ??__Fg_FACILITY_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FA90-0x0044FAA4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_facility_icon_sprites() {
    VectorDtorIterator(g_FACILITY_ICON_SPRITES, 0x2C, 70, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SECRET_PROJECT_ICON_SPRITES.
ORIGINAL: 0x0044FAE0
// name      ??__Fg_SECRET_PROJECT_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FAE0-0x0044FAF4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_secret_project_icon_sprites() {
    VectorDtorIterator(g_SECRET_PROJECT_ICON_SPRITES, 0x2C, 64, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_MP_COMBO_ARROW_SPRITES.
ORIGINAL: 0x0044FB30
// name      ??__Fg_IFACE_MP_COMBO_ARROW_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FB30-0x0044FB44
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_mp_combo_arrow_sprites() {
    VectorDtorIterator(g_IFACE_MP_COMBO_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_ARROW_ICON_SPRITES.
ORIGINAL: 0x0044FB80
// name      ??__Fg_SCROLL_BAR_ARROW_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FB80-0x0044FB94
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES.
ORIGINAL: 0x0044FBD0
// name      ??__Fg_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FBD0-0x0044FBE4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_arrow_icon_sprites() {
    VectorDtorIterator(g_SCROLL_BAR_SMALL_ARROW_ICON_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_FILLER_ICON_SPRITES.
ORIGINAL: 0x0044FC10
// name      ??__Fg_SCROLL_BAR_FILLER_ICON_SPRITES@@YAXXZ
// size      10 bytes
// spans     0x0044FC10-0x0044FC1A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_filler_icon_sprites() {
    g_SCROLL_BAR_FILLER_ICON_SPRITES->close();
}

/*
Purpose: Atexit teardown thunk for g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE.
ORIGINAL: 0x0044FC40
// name      ??__Fg_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FC40-0x0044FC4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_scroll_bar_small_filler_icon_sprite() {
    g_SCROLL_BAR_SMALL_FILLER_ICON_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_LOCK_SPRITES.
ORIGINAL: 0x0044FC80
// name      ??__Fg_IFACE_LOCK_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0044FC80-0x0044FC94
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_lock_sprites() {
    VectorDtorIterator(g_IFACE_LOCK_SPRITES, 0x2C, 2, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR01.
ORIGINAL: 0x0044FCD0
// name      ??__Fg_UNUSED_SPRITES_VAR01@@YAXXZ
// size      20 bytes
// spans     0x0044FCD0-0x0044FCE4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var01() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR01, 0x2C, 8, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE.
ORIGINAL: 0x0044FD10
// name      ??__Fg_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FD10-0x0044FD1A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE.
ORIGINAL: 0x0044FD40
// name      ??__Fg_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FD40-0x0044FD4A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE.
ORIGINAL: 0x0044FD70
// name      ??__Fg_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FD70-0x0044FD7A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE.
ORIGINAL: 0x0044FDA0
// name      ??__Fg_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FDA0-0x0044FDAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE.
ORIGINAL: 0x0044FDD0
// name      ??__Fg_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FDD0-0x0044FDDA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE.
ORIGINAL: 0x0044FE00
// name      ??__Fg_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FE00-0x0044FE0A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_mid_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_MID_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE.
ORIGINAL: 0x0044FE30
// name      ??__Fg_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FE30-0x0044FE3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_top_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_TOP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE.
ORIGINAL: 0x0044FE60
// name      ??__Fg_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FE60-0x0044FE6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_bot_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_BOT_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE.
ORIGINAL: 0x0044FE90
// name      ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FE90-0x0044FE9A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_mid_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_MID_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE.
ORIGINAL: 0x0044FEC0
// name      ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FEC0-0x0044FECA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_left_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_LEFT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE.
ORIGINAL: 0x0044FEF0
// name      ??__Fg_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE@@YAXXZ
// size      10 bytes
// spans     0x0044FEF0-0x0044FEFA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_general_windows_noncap_right_sprite() {
    g_IFACE_GENERAL_WINDOWS_NONCAP_RIGHT_SPRITE->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR23.
ORIGINAL: 0x0044FF20
// name      ??__Fg_UNUSED_SPRITE_VAR23@@YAXXZ
// size      10 bytes
// spans     0x0044FF20-0x0044FF2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var23() {
    g_UNUSED_SPRITE_VAR23->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR24.
ORIGINAL: 0x0044FF50
// name      ??__Fg_UNUSED_SPRITE_VAR24@@YAXXZ
// size      10 bytes
// spans     0x0044FF50-0x0044FF5A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var24() {
    g_UNUSED_SPRITE_VAR24->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR25.
ORIGINAL: 0x0044FF80
// name      ??__Fg_UNUSED_SPRITE_VAR25@@YAXXZ
// size      10 bytes
// spans     0x0044FF80-0x0044FF8A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var25() {
    g_UNUSED_SPRITE_VAR25->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR26.
ORIGINAL: 0x0044FFB0
// name      ??__Fg_UNUSED_SPRITE_VAR26@@YAXXZ
// size      10 bytes
// spans     0x0044FFB0-0x0044FFBA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var26() {
    g_UNUSED_SPRITE_VAR26->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR27.
ORIGINAL: 0x0044FFE0
// name      ??__Fg_UNUSED_SPRITE_VAR27@@YAXXZ
// size      10 bytes
// spans     0x0044FFE0-0x0044FFEA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var27() {
    g_UNUSED_SPRITE_VAR27->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR28.
ORIGINAL: 0x00450010
// name      ??__Fg_UNUSED_SPRITE_VAR28@@YAXXZ
// size      10 bytes
// spans     0x00450010-0x0045001A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var28() {
    g_UNUSED_SPRITE_VAR28->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR29.
ORIGINAL: 0x00450040
// name      ??__Fg_UNUSED_SPRITE_VAR29@@YAXXZ
// size      10 bytes
// spans     0x00450040-0x0045004A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var29() {
    g_UNUSED_SPRITE_VAR29->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR30.
ORIGINAL: 0x00450070
// name      ??__Fg_UNUSED_SPRITE_VAR30@@YAXXZ
// size      10 bytes
// spans     0x00450070-0x0045007A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var30() {
    g_UNUSED_SPRITE_VAR30->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR31.
ORIGINAL: 0x004500A0
// name      ??__Fg_UNUSED_SPRITE_VAR31@@YAXXZ
// size      10 bytes
// spans     0x004500A0-0x004500AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var31() {
    g_UNUSED_SPRITE_VAR31->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR32.
ORIGINAL: 0x004500D0
// name      ??__Fg_UNUSED_SPRITE_VAR32@@YAXXZ
// size      10 bytes
// spans     0x004500D0-0x004500DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var32() {
    g_UNUSED_SPRITE_VAR32->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR33.
ORIGINAL: 0x00450100
// name      ??__Fg_UNUSED_SPRITE_VAR33@@YAXXZ
// size      10 bytes
// spans     0x00450100-0x0045010A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var33() {
    g_UNUSED_SPRITE_VAR33->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR34.
ORIGINAL: 0x00450130
// name      ??__Fg_UNUSED_SPRITE_VAR34@@YAXXZ
// size      10 bytes
// spans     0x00450130-0x0045013A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var34() {
    g_UNUSED_SPRITE_VAR34->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR35.
ORIGINAL: 0x00450160
// name      ??__Fg_UNUSED_SPRITE_VAR35@@YAXXZ
// size      10 bytes
// spans     0x00450160-0x0045016A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var35() {
    g_UNUSED_SPRITE_VAR35->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR36.
ORIGINAL: 0x00450190
// name      ??__Fg_UNUSED_SPRITE_VAR36@@YAXXZ
// size      10 bytes
// spans     0x00450190-0x0045019A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var36() {
    g_UNUSED_SPRITE_VAR36->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR37.
ORIGINAL: 0x004501C0
// name      ??__Fg_UNUSED_SPRITE_VAR37@@YAXXZ
// size      10 bytes
// spans     0x004501C0-0x004501CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var37() {
    g_UNUSED_SPRITE_VAR37->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR38.
ORIGINAL: 0x004501F0
// name      ??__Fg_UNUSED_SPRITE_VAR38@@YAXXZ
// size      10 bytes
// spans     0x004501F0-0x004501FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var38() {
    g_UNUSED_SPRITE_VAR38->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR39.
ORIGINAL: 0x00450220
// name      ??__Fg_UNUSED_SPRITE_VAR39@@YAXXZ
// size      10 bytes
// spans     0x00450220-0x0045022A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var39() {
    g_UNUSED_SPRITE_VAR39->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR40.
ORIGINAL: 0x00450250
// name      ??__Fg_UNUSED_SPRITE_VAR40@@YAXXZ
// size      10 bytes
// spans     0x00450250-0x0045025A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var40() {
    g_UNUSED_SPRITE_VAR40->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR41.
ORIGINAL: 0x00450280
// name      ??__Fg_UNUSED_SPRITE_VAR41@@YAXXZ
// size      10 bytes
// spans     0x00450280-0x0045028A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var41() {
    g_UNUSED_SPRITE_VAR41->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR42.
ORIGINAL: 0x004502B0
// name      ??__Fg_UNUSED_SPRITE_VAR42@@YAXXZ
// size      10 bytes
// spans     0x004502B0-0x004502BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var42() {
    g_UNUSED_SPRITE_VAR42->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR43.
ORIGINAL: 0x004502E0
// name      ??__Fg_UNUSED_SPRITE_VAR43@@YAXXZ
// size      10 bytes
// spans     0x004502E0-0x004502EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var43() {
    g_UNUSED_SPRITE_VAR43->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR44.
ORIGINAL: 0x00450310
// name      ??__Fg_UNUSED_SPRITE_VAR44@@YAXXZ
// size      10 bytes
// spans     0x00450310-0x0045031A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var44() {
    g_UNUSED_SPRITE_VAR44->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR45.
ORIGINAL: 0x00450340
// name      ??__Fg_UNUSED_SPRITE_VAR45@@YAXXZ
// size      10 bytes
// spans     0x00450340-0x0045034A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var45() {
    g_UNUSED_SPRITE_VAR45->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR46.
ORIGINAL: 0x00450370
// name      ??__Fg_UNUSED_SPRITE_VAR46@@YAXXZ
// size      10 bytes
// spans     0x00450370-0x0045037A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var46() {
    g_UNUSED_SPRITE_VAR46->close();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_TECH_TREE_ARROW_SPRITES.
ORIGINAL: 0x004503B0
// name      ??__Fg_IFACE_TECH_TREE_ARROW_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x004503B0-0x004503C4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_tech_tree_arrow_sprites() {
    VectorDtorIterator(g_IFACE_TECH_TREE_ARROW_SPRITES, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR47.
ORIGINAL: 0x004503F0
// name      ??__Fg_UNUSED_SPRITE_VAR47@@YAXXZ
// size      10 bytes
// spans     0x004503F0-0x004503FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var47() {
    g_UNUSED_SPRITE_VAR47->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR48.
ORIGINAL: 0x00450420
// name      ??__Fg_UNUSED_SPRITE_VAR48@@YAXXZ
// size      10 bytes
// spans     0x00450420-0x0045042A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var48() {
    g_UNUSED_SPRITE_VAR48->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR49.
ORIGINAL: 0x00450450
// name      ??__Fg_UNUSED_SPRITE_VAR49@@YAXXZ
// size      10 bytes
// spans     0x00450450-0x0045045A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var49() {
    g_UNUSED_SPRITE_VAR49->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR50.
ORIGINAL: 0x00450480
// name      ??__Fg_UNUSED_SPRITE_VAR50@@YAXXZ
// size      10 bytes
// spans     0x00450480-0x0045048A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var50() {
    g_UNUSED_SPRITE_VAR50->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR51.
ORIGINAL: 0x004504B0
// name      ??__Fg_UNUSED_SPRITE_VAR51@@YAXXZ
// size      10 bytes
// spans     0x004504B0-0x004504BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var51() {
    g_UNUSED_SPRITE_VAR51->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR52.
ORIGINAL: 0x004504E0
// name      ??__Fg_UNUSED_SPRITE_VAR52@@YAXXZ
// size      10 bytes
// spans     0x004504E0-0x004504EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var52() {
    g_UNUSED_SPRITE_VAR52->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR53.
ORIGINAL: 0x00450510
// name      ??__Fg_UNUSED_SPRITE_VAR53@@YAXXZ
// size      10 bytes
// spans     0x00450510-0x0045051A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var53() {
    g_UNUSED_SPRITE_VAR53->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR54.
ORIGINAL: 0x00450540
// name      ??__Fg_UNUSED_SPRITE_VAR54@@YAXXZ
// size      10 bytes
// spans     0x00450540-0x0045054A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var54() {
    g_UNUSED_SPRITE_VAR54->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR55.
ORIGINAL: 0x00450570
// name      ??__Fg_UNUSED_SPRITE_VAR55@@YAXXZ
// size      10 bytes
// spans     0x00450570-0x0045057A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var55() {
    g_UNUSED_SPRITE_VAR55->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR56.
ORIGINAL: 0x004505A0
// name      ??__Fg_UNUSED_SPRITE_VAR56@@YAXXZ
// size      10 bytes
// spans     0x004505A0-0x004505AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var56() {
    g_UNUSED_SPRITE_VAR56->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR57.
ORIGINAL: 0x004505D0
// name      ??__Fg_UNUSED_SPRITE_VAR57@@YAXXZ
// size      10 bytes
// spans     0x004505D0-0x004505DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var57() {
    g_UNUSED_SPRITE_VAR57->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR58.
ORIGINAL: 0x00450600
// name      ??__Fg_UNUSED_SPRITE_VAR58@@YAXXZ
// size      10 bytes
// spans     0x00450600-0x0045060A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var58() {
    g_UNUSED_SPRITE_VAR58->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR59.
ORIGINAL: 0x00450630
// name      ??__Fg_UNUSED_SPRITE_VAR59@@YAXXZ
// size      10 bytes
// spans     0x00450630-0x0045063A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var59() {
    g_UNUSED_SPRITE_VAR59->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR60.
ORIGINAL: 0x00450660
// name      ??__Fg_UNUSED_SPRITE_VAR60@@YAXXZ
// size      10 bytes
// spans     0x00450660-0x0045066A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var60() {
    g_UNUSED_SPRITE_VAR60->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR61.
ORIGINAL: 0x00450690
// name      ??__Fg_UNUSED_SPRITE_VAR61@@YAXXZ
// size      10 bytes
// spans     0x00450690-0x0045069A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var61() {
    g_UNUSED_SPRITE_VAR61->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR62.
ORIGINAL: 0x004506C0
// name      ??__Fg_UNUSED_SPRITE_VAR62@@YAXXZ
// size      10 bytes
// spans     0x004506C0-0x004506CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var62() {
    g_UNUSED_SPRITE_VAR62->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR63.
ORIGINAL: 0x004506F0
// name      ??__Fg_UNUSED_SPRITE_VAR63@@YAXXZ
// size      10 bytes
// spans     0x004506F0-0x004506FA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var63() {
    g_UNUSED_SPRITE_VAR63->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR64.
ORIGINAL: 0x00450720
// name      ??__Fg_UNUSED_SPRITE_VAR64@@YAXXZ
// size      10 bytes
// spans     0x00450720-0x0045072A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var64() {
    g_UNUSED_SPRITE_VAR64->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR65.
ORIGINAL: 0x00450750
// name      ??__Fg_UNUSED_SPRITE_VAR65@@YAXXZ
// size      10 bytes
// spans     0x00450750-0x0045075A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var65() {
    g_UNUSED_SPRITE_VAR65->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR66.
ORIGINAL: 0x00450780
// name      ??__Fg_UNUSED_SPRITE_VAR66@@YAXXZ
// size      10 bytes
// spans     0x00450780-0x0045078A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var66() {
    g_UNUSED_SPRITE_VAR66->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR67.
ORIGINAL: 0x004507B0
// name      ??__Fg_UNUSED_SPRITE_VAR67@@YAXXZ
// size      10 bytes
// spans     0x004507B0-0x004507BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var67() {
    g_UNUSED_SPRITE_VAR67->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR68.
ORIGINAL: 0x004507E0
// name      ??__Fg_UNUSED_SPRITE_VAR68@@YAXXZ
// size      10 bytes
// spans     0x004507E0-0x004507EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var68() {
    g_UNUSED_SPRITE_VAR68->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR69.
ORIGINAL: 0x00450810
// name      ??__Fg_UNUSED_SPRITE_VAR69@@YAXXZ
// size      10 bytes
// spans     0x00450810-0x0045081A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var69() {
    g_UNUSED_SPRITE_VAR69->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR70.
ORIGINAL: 0x00450840
// name      ??__Fg_UNUSED_SPRITE_VAR70@@YAXXZ
// size      10 bytes
// spans     0x00450840-0x0045084A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var70() {
    g_UNUSED_SPRITE_VAR70->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR71.
ORIGINAL: 0x00450870
// name      ??__Fg_UNUSED_SPRITE_VAR71@@YAXXZ
// size      10 bytes
// spans     0x00450870-0x0045087A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var71() {
    g_UNUSED_SPRITE_VAR71->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR72.
ORIGINAL: 0x004508A0
// name      ??__Fg_UNUSED_SPRITE_VAR72@@YAXXZ
// size      10 bytes
// spans     0x004508A0-0x004508AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var72() {
    g_UNUSED_SPRITE_VAR72->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR73.
ORIGINAL: 0x004508D0
// name      ??__Fg_UNUSED_SPRITE_VAR73@@YAXXZ
// size      10 bytes
// spans     0x004508D0-0x004508DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var73() {
    g_UNUSED_SPRITE_VAR73->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR74.
ORIGINAL: 0x00450900
// name      ??__Fg_UNUSED_SPRITE_VAR74@@YAXXZ
// size      10 bytes
// spans     0x00450900-0x0045090A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var74() {
    g_UNUSED_SPRITE_VAR74->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR02.
ORIGINAL: 0x00450940
// name      ??__Fg_UNUSED_SPRITES_VAR02@@YAXXZ
// size      20 bytes
// spans     0x00450940-0x00450954
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var02() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR02, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR03.
ORIGINAL: 0x00450990
// name      ??__Fg_UNUSED_SPRITES_VAR03@@YAXXZ
// size      20 bytes
// spans     0x00450990-0x004509A4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var03() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR03, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR04.
ORIGINAL: 0x004509E0
// name      ??__Fg_UNUSED_SPRITES_VAR04@@YAXXZ
// size      20 bytes
// spans     0x004509E0-0x004509F4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var04() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR04, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR05.
ORIGINAL: 0x00450A30
// name      ??__Fg_UNUSED_SPRITES_VAR05@@YAXXZ
// size      20 bytes
// spans     0x00450A30-0x00450A44
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var05() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR05, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR06.
ORIGINAL: 0x00450A80
// name      ??__Fg_UNUSED_SPRITES_VAR06@@YAXXZ
// size      20 bytes
// spans     0x00450A80-0x00450A94
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var06() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR06, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR07.
ORIGINAL: 0x00450AD0
// name      ??__Fg_UNUSED_SPRITES_VAR07@@YAXXZ
// size      20 bytes
// spans     0x00450AD0-0x00450AE4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var07() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR07, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR08.
ORIGINAL: 0x00450B20
// name      ??__Fg_UNUSED_SPRITES_VAR08@@YAXXZ
// size      20 bytes
// spans     0x00450B20-0x00450B34
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var08() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR08, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR09.
ORIGINAL: 0x00450B70
// name      ??__Fg_UNUSED_SPRITES_VAR09@@YAXXZ
// size      20 bytes
// spans     0x00450B70-0x00450B84
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var09() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR09, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITES_VAR10.
ORIGINAL: 0x00450BC0
// name      ??__Fg_UNUSED_SPRITES_VAR10@@YAXXZ
// size      20 bytes
// spans     0x00450BC0-0x00450BD4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprites_var10() {
    VectorDtorIterator(g_UNUSED_SPRITES_VAR10, 0x2C, 3, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR75.
ORIGINAL: 0x00450C00
// name      ??__Fg_UNUSED_SPRITE_VAR75@@YAXXZ
// size      10 bytes
// spans     0x00450C00-0x00450C0A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var75() {
    g_UNUSED_SPRITE_VAR75->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR76.
ORIGINAL: 0x00450C30
// name      ??__Fg_UNUSED_SPRITE_VAR76@@YAXXZ
// size      10 bytes
// spans     0x00450C30-0x00450C3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var76() {
    g_UNUSED_SPRITE_VAR76->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR77.
ORIGINAL: 0x00450C60
// name      ??__Fg_UNUSED_SPRITE_VAR77@@YAXXZ
// size      10 bytes
// spans     0x00450C60-0x00450C6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var77() {
    g_UNUSED_SPRITE_VAR77->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR78.
ORIGINAL: 0x00450C90
// name      ??__Fg_UNUSED_SPRITE_VAR78@@YAXXZ
// size      10 bytes
// spans     0x00450C90-0x00450C9A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var78() {
    g_UNUSED_SPRITE_VAR78->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR79.
ORIGINAL: 0x00450CC0
// name      ??__Fg_UNUSED_SPRITE_VAR79@@YAXXZ
// size      10 bytes
// spans     0x00450CC0-0x00450CCA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var79() {
    g_UNUSED_SPRITE_VAR79->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR80.
ORIGINAL: 0x00450CF0
// name      ??__Fg_UNUSED_SPRITE_VAR80@@YAXXZ
// size      10 bytes
// spans     0x00450CF0-0x00450CFA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var80() {
    g_UNUSED_SPRITE_VAR80->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR81.
ORIGINAL: 0x00450D20
// name      ??__Fg_UNUSED_SPRITE_VAR81@@YAXXZ
// size      10 bytes
// spans     0x00450D20-0x00450D2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var81() {
    g_UNUSED_SPRITE_VAR81->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR82.
ORIGINAL: 0x00450D50
// name      ??__Fg_UNUSED_SPRITE_VAR82@@YAXXZ
// size      10 bytes
// spans     0x00450D50-0x00450D5A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var82() {
    g_UNUSED_SPRITE_VAR82->close();
}

/*
Purpose: Atexit teardown thunk for g_UNUSED_SPRITE_VAR83.
ORIGINAL: 0x00450D80
// name      ??__Fg_UNUSED_SPRITE_VAR83@@YAXXZ
// size      10 bytes
// spans     0x00450D80-0x00450D8A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_unused_sprite_var83() {
    g_UNUSED_SPRITE_VAR83->close();
}

/*
Purpose: Atexit teardown thunk for g_BASEWIN_SPRITES.
ORIGINAL: 0x00450DC0
// name      ??__Fg_BASEWIN_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x00450DC0-0x00450DD4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_basewin_sprites() {
    VectorDtorIterator(g_BASEWIN_SPRITES, 0x2C, 27, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_INFOWIN.
ORIGINAL: 0x004562E0
// name      ??__Fg_INFOWIN@@YAXXZ
// size      10 bytes
// spans     0x004562E0-0x004562EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_infowin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AD2A0))->*InfoWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE.
ORIGINAL: 0x00459580
// name      ??__Fg_MAININTERFACE@@YAXXZ
// size      10 bytes
// spans     0x00459580-0x0045958A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface() {
    (ORIGINAL(reinterpret_cast<void *>(0x007AE820))->*MainInterfaceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MAININTERFACE_WAVE.
ORIGINAL: 0x004595B0
// name      ??__Fg_MAININTERFACE_WAVE@@YAXXZ
// size      10 bytes
// spans     0x004595B0-0x004595BA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_maininterface_wave() {
    (ORIGINAL(g_MAININTERFACE_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_JACKAL_FONT.
ORIGINAL: 0x0045F940
// name      ??__Fg_JACKAL_FONT@@YAXXZ
// size      10 bytes
// spans     0x0045F940-0x0045F94A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_jackal_font() {
    g_JACKAL_FONT->~Font();
}

/*
Purpose: Atexit teardown thunk for g_MESSAGEWIN.
ORIGINAL: 0x00471340
// name      ??__Fg_MESSAGEWIN@@YAXXZ
// size      10 bytes
// spans     0x00471340-0x0047134A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_messagewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F67F8))->*MessageWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_IFACE_GREEN_RIGHT_ARROW_SPRITE.
ORIGINAL: 0x00471380
// name      ??__Fg_IFACE_GREEN_RIGHT_ARROW_SPRITE@@YAXXZ
// size      20 bytes
// spans     0x00471380-0x00471394
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_iface_green_right_arrow_sprite() {
    VectorDtorIterator(g_IFACE_GREEN_RIGHT_ARROW_SPRITE, 0x2C, 1, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_MONUWIN.
ORIGINAL: 0x00472220
// name      ??__Fg_MONUWIN@@YAXXZ
// size      10 bytes
// spans     0x00472220-0x0047222A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_monuwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007F9F58))->*MonuWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN.
ORIGINAL: 0x00477E30
// name      ??__Fg_MULTIWIN@@YAXXZ
// size      10 bytes
// spans     0x00477E30-0x00477E3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FD648))->*MultiWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIWIN_WAVE.
ORIGINAL: 0x00477E60
// name      ??__Fg_MULTIWIN_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00477E60-0x00477E6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multiwin_wave() {
    (ORIGINAL(g_MULTIWIN_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG1.
ORIGINAL: 0x0047A790
// name      ??__Fg_NETMSG1@@YAXXZ
// size      10 bytes
// spans     0x0047A790-0x0047A79A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg1() {
    (ORIGINAL(reinterpret_cast<void *>(0x00805338))->*NetMsgDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NETMSG2.
ORIGINAL: 0x0047A7C0
// name      ??__Fg_NETMSG2@@YAXXZ
// size      10 bytes
// spans     0x0047A7C0-0x0047A7CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netmsg2() {
    (ORIGINAL(reinterpret_cast<void *>(0x007FFF80))->*NetMsgDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NETWIN.
ORIGINAL: 0x0047B010
// name      ??__Fg_NETWIN@@YAXXZ
// size      10 bytes
// spans     0x0047B010-0x0047B01A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x0080A6F8))->*NetWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_NEWTECHWIN.
ORIGINAL: 0x00483870
// name      ??__Fg_NEWTECHWIN@@YAXXZ
// size      10 bytes
// spans     0x00483870-0x0048387A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_newtechwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00811E40))->*NewTechWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PICKWIN.
ORIGINAL: 0x00488770
// name      ??__Fg_PICKWIN@@YAXXZ
// size      10 bytes
// spans     0x00488770-0x0048877A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_pickwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00822718))->*PickWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN.
ORIGINAL: 0x0048D510
// name      ??__Fg_PREFWIN@@YAXXZ
// size      10 bytes
// spans     0x0048D510-0x0048D51A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008578D8))->*PrefWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PREFWIN_BUTTONGROUP.
ORIGINAL: 0x0048D540
// name      ??__Fg_PREFWIN_BUTTONGROUP@@YAXXZ
// size      10 bytes
// spans     0x0048D540-0x0048D54A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_prefwin_buttongroup() {
    g_PREFWIN_BUTTONGROUP->close();
}

/*
Purpose: Atexit teardown thunk for g_QUAYLEWIN.
ORIGINAL: 0x00495190
// name      ??__Fg_QUAYLEWIN@@YAXXZ
// size      10 bytes
// spans     0x00495190-0x0049519A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_quaylewin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00872CB0))->*QuayleWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_REPORTIF.
ORIGINAL: 0x00496920
// name      ??__Fg_REPORTIF@@YAXXZ
// size      10 bytes
// spans     0x00496920-0x0049692A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportif() {
    (ORIGINAL(reinterpret_cast<void *>(0x00885F38))->*ReportIfDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_REPORTWIN.
ORIGINAL: 0x00496950
// name      ??__Fg_REPORTWIN@@YAXXZ
// size      10 bytes
// spans     0x00496950-0x0049695A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_reportwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x00876478))->*ReportWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for SocialWinParent.
ORIGINAL: 0x004AE9D0
// name      ??__FSocialWinParent@@YAXXZ
// size      10 bytes
// spans     0x004AE9D0-0x004AE9DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_socialwinparent() {
    (ORIGINAL(reinterpret_cast<void *>(0x008A6270))->*SocialWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_STATUSWIN.
ORIGINAL: 0x004B3FC0
// name      ??__Fg_STATUSWIN@@YAXXZ
// size      10 bytes
// spans     0x004B3FC0-0x004B3FCA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_statuswin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C5568))->*StatusWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_TUTWIN.
ORIGINAL: 0x004BA5E0
// name      ??__Fg_TUTWIN@@YAXXZ
// size      10 bytes
// spans     0x004BA5E0-0x004BA5EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_tutwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008C6E68))->*TutWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_CAVIAR.
ORIGINAL: 0x004BF700
// name      ??__Fg_VEHDRAW_CAVIAR@@YAXXZ
// size      10 bytes
// spans     0x004BF700-0x004BF70A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_caviar() {
    (ORIGINAL(reinterpret_cast<void *>(0x008CC828))->*CaviarCloseTarget)();
}

/*
Purpose: Atexit teardown thunk for g_VEHDRAW_BUFFER.
ORIGINAL: 0x004BF730
// name      ??__Fg_VEHDRAW_BUFFER@@YAXXZ
// size      10 bytes
// spans     0x004BF730-0x004BF73A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_vehdraw_buffer() {
    g_VEHDRAW_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for g_WORLDWIN.
ORIGINAL: 0x004C38D0
// name      ??__Fg_WORLDWIN@@YAXXZ
// size      10 bytes
// spans     0x004C38D0-0x004C38DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_worldwin() {
    (ORIGINAL(reinterpret_cast<void *>(0x008E9F60))->*WorldWinDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_DEVICE.
ORIGINAL: 0x004C5C70
// name      ??__Fg_WAVE_DEVICE@@YAXXZ
// size      10 bytes
// spans     0x004C5C70-0x004C5C7A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D978))->*Wave_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MIDI_DEVICE.
ORIGINAL: 0x004C5CA0
// name      ??__Fg_MIDI_DEVICE@@YAXXZ
// size      10 bytes
// spans     0x004C5CA0-0x004C5CAA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_midi_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090D950))->*Midi_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_IN_DEVICE.
ORIGINAL: 0x004C5CD0
// name      ??__Fg_WAVE_IN_DEVICE@@YAXXZ
// size      10 bytes
// spans     0x004C5CD0-0x004C5CDA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_in_device() {
    (ORIGINAL(reinterpret_cast<void *>(0x0090DB50))->*Wave_In_DeviceDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CONSOLE_TIMER.
ORIGINAL: 0x0050E9A0
// name      ??__Fg_CONSOLE_TIMER@@YAXXZ
// size      10 bytes
// spans     0x0050E9A0-0x0050E9AA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_console_timer() {
    (ORIGINAL(reinterpret_cast<void *>(0x00939E88))->*TimeDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_CURSOR_SPRITES.
ORIGINAL: 0x0052DAF0
// name      ??__Fg_CURSOR_SPRITES@@YAXXZ
// size      20 bytes
// spans     0x0052DAF0-0x0052DB04
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_cursor_sprites() {
    VectorDtorIterator(g_CURSOR_SPRITES, 0x2C, 12, SpriteElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_NETDAEMON.
ORIGINAL: 0x0052DB30
// name      ??__Fg_NETDAEMON@@YAXXZ
// size      10 bytes
// spans     0x0052DB30-0x0052DB3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_netdaemon() {
    (ORIGINAL(reinterpret_cast<void *>(0x0093CD90))->*NetDaemonDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL2.
ORIGINAL: 0x00559250
// name      ??__Fg_FONTQUEUE_VAL2@@YAXXZ
// size      10 bytes
// spans     0x00559250-0x0055925A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val2() {
    g_FONTQUEUE_VAL2->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_FONTQUEUE_VAL1.
ORIGINAL: 0x00559280
// name      ??__Fg_FONTQUEUE_VAL1@@YAXXZ
// size      10 bytes
// spans     0x00559280-0x0055928A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fontqueue_val1() {
    g_FONTQUEUE_VAL1->~FontQueue();
}

/*
Purpose: Atexit teardown thunk for g_TOP_MENU_WAVE.
ORIGINAL: 0x00584D20
// name      ??__Fg_TOP_MENU_WAVE@@YAXXZ
// size      10 bytes
// spans     0x00584D20-0x00584D2A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_top_menu_wave() {
    (ORIGINAL(g_TOP_MENU_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_FONTS.
ORIGINAL: 0x005882D0
// name      ??__Fg_FONTS@@YAXXZ
// size      20 bytes
// spans     0x005882D0-0x005882E4
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_fonts() {
    VectorDtorIterator(g_FONTS, 0x28, 48, FontElementTeardown);
}

/*
Purpose: Atexit teardown thunk for g_CRASH_LANDING_WAVE.
ORIGINAL: 0x005AE110
// name      ??__Fg_CRASH_LANDING_WAVE@@YAXXZ
// size      10 bytes
// spans     0x005AE110-0x005AE11A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_crash_landing_wave() {
    (ORIGINAL(g_CRASH_LANDING_WAVE)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_WAVE_GENERAL.
ORIGINAL: 0x005B9C30
// name      ??__Fg_WAVE_GENERAL@@YAXXZ
// size      10 bytes
// spans     0x005B9C30-0x005B9C3A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_wave_general() {
    (ORIGINAL(g_WAVE_GENERAL)->*WaveOriginalDestructor)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE1.
ORIGINAL: 0x005BEC60
// name      ??__Fg_PALETTE1@@YAXXZ
// size      10 bytes
// spans     0x005BEC60-0x005BEC6A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette1() {
    (ORIGINAL(reinterpret_cast<void *>(0x0094C590))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_PALETTE2.
ORIGINAL: 0x005BEC90
// name      ??__Fg_PALETTE2@@YAXXZ
// size      10 bytes
// spans     0x005BEC90-0x005BEC9A
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_palette2() {
    (ORIGINAL(reinterpret_cast<void *>(0x009523A0))->*PaletteDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_MULTIDEBUG.
ORIGINAL: 0x005C97E0
// name      ??__Fg_MULTIDEBUG@@YAXXZ
// size      10 bytes
// spans     0x005C97E0-0x005C97EA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_multidebug() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B22F0))->*MultiDebugDtorTarget)();
}

/*
Purpose: Atexit teardown thunk for g_BUFFER.
ORIGINAL: 0x005E37D0
// name      ??__Fg_BUFFER@@YAXXZ
// size      10 bytes
// spans     0x005E37D0-0x005E37DA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_buffer() {
    g_BUFFER->destroy();
}

/*
Purpose: Atexit teardown thunk for TxtIndex.
ORIGINAL: 0x005FD510
// name      ??__FTxtIndex@@YAXXZ
// size      23 bytes
// spans     0x005FD510-0x005FD527
// prototype 
// callers   0   call targets   1
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_txtindex() {
    VectorDtorIterator(TxtIndexGlobal, 0x118, 4, TextIndexElementTeardown);
}

/*
Purpose: Atexit teardown thunk for StringTable.
ORIGINAL: 0x006168C0
// name      ??__FStringTable@@YAXXZ
// size      10 bytes
// spans     0x006168C0-0x006168CA
// prototype 
// callers   0   call targets   0
Return Value: n/a
Status: Complete
*/
void __cdecl destroy_stringtable() {
    (ORIGINAL(reinterpret_cast<void *>(0x009B90D8))->*StringsDtorTarget)();
}
