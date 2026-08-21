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

#pragma once

#include "original_seam.h"

class Ambience;
class Buffer;
class ButtonGroup;
class Caviar;
class CaviarData;
class FX;
class Font;
class Sprite;
class Strings;
class TextIndex;
class Texture;
class Time;
class Wave;
class Wave_Device;

/*
 * Dynamic-initializer thunks
 *
 * One ??__E thunk per dynamically-initialized global: construct the object at
 * its fixed address through the recovered constructor, then register its
 * catalogued ??__F teardown with `atexit`.
 *
 * IT USED TO GO THROUGH A `GameAtexit` SEAM, so that the GAME's CRT exit list
 * owned the registration and teardown order stayed inside its LIFO walk. That
 * mattered when this tree was staged as a DLL inside the original process,
 * where the two CRTs keep separate lists. There is no such build any more -
 * CMakeLists.txt produces one executable and the staging tools are gone - and
 * this build links the same VC6 CRT the image statically linked, so `atexit`
 * IS 0x00645398's counterpart. The seam cost every thunk a `call dword ptr`
 * where the image has `call rel32`. Globals shared with the teardown side stay
 * declared in atexit_thunks.h; only the ones the teardown side does not bind
 * appear here.
 */

typedef void(__cdecl func_atexit_callback)();

#include "vector_teardown.h"
extern const void *const BufferElementCtor;
extern const void *const CaviarDataElementCtor;
extern const void *const FontElementCtor;
extern const void *const SpriteElementCtor;
extern const void *const TextIndexElementCtor;
extern const void *const TextureElementCtor;
extern const void *const WaveElementCtor;

// Per-kind scalar construction seams, defaulting to
// source-owned trampolines over the recovered
// constructors. Rebindable because those constructors
// reach their own deeper seams, whose defaults are
// unmapped outside the game process.
extern func_thiscall_teardown AmbienceInitCtor;
extern func_thiscall_teardown BufferInitCtor;
extern func_thiscall_teardown ButtonGroupInitCtor;
extern func_thiscall_teardown CaviarDataInitCtor;
extern func_thiscall_teardown CaviarInitCtor;
extern func_thiscall_teardown FXInitCtor;
extern func_thiscall_teardown FontInitCtor;
extern func_thiscall_teardown SpriteInitCtor;
extern func_thiscall_teardown StringsInitCtor;
extern func_thiscall_teardown TextureInitCtor;
extern func_thiscall_teardown TimeInitCtor;
extern func_thiscall_teardown WaveDeviceInitCtor;
extern func_thiscall_teardown WaveInitCtor;

typedef void (OriginalObject::*func_opaque_ctor_i)(int a0);
typedef void (OriginalObject::*func_opaque_ctor_iii)(int a0, int a1, int a2);

// Opaque-storage construction seams. These globals' classes
// are not modelled in source, so the thunk hands the storage
// address straight to the original constructor through a seam
// rather than naming a type. Seams are deduped on the ADDRESS
// they default to; one already bound at the same address
// elsewhere is reused, never redefined here.
extern func_opaque_ctor_i MapWinCtorTarget;   // 0x004626E0
extern func_opaque_ctor_iii NetMsgCtorTarget;   // 0x0047ACF0
extern func_opaque_ctor_i PlanWinCtorTarget;   // 0x0048BCD0
extern func_opaque_ctor_i ConsoleCtorTarget;   // 0x0050F460

Sprite *const g_NEWTECHWIN_SPRITES = (Sprite *)0x00822610;
Caviar *const g_VEHDRAW_CAVIAR = (Caviar *)0x008CC828;
Wave_Device *const g_WAVE_DEVICE = (Wave_Device *)0x0090D978;
Time *const g_CONSOLE_TIMER = (Time *)0x00939E88;
Sprite *const g_BUFFER_SPRITE = (Sprite *)0x009B3A50;
Buffer *const g_WIN_BUFFER = (Buffer *)0x009B6F08;
Sprite *const g_RADIOBUTTON_SPRITE_1 = (Sprite *)0x009B8EF8;
Sprite *const g_RADIOBUTTON_SPRITE_2 = (Sprite *)0x009B8F28;
// DEFINED HERE, not declared. A namespace-scope `const` pointer has internal
// linkage, so every translation unit folds it to the immediate the image uses
// - `mov ecx, 0x9b8f60` - where an `extern` variable costs a load from a slot
// the image never reads.
Sprite *const g_CHECKBOX_SPRITE_1 = (Sprite *)0x009B8F60;
Sprite *const g_CHECKBOX_SPRITE_2 = (Sprite *)0x009B8F90;
Sprite *const g_FILEWIN_SPRITE_1 = (Sprite *)0x009B9048;
Sprite *const g_FILEWIN_SPRITE_2 = (Sprite *)0x009B9018;
Sprite *const g_FILEWIN_SPRITE_3 = (Sprite *)0x009B9078;
Buffer *const g_CAVIAR_BUFFER_1 = (Buffer *)0x009B9108;
Buffer *const g_CAVIAR_BUFFER_2 = (Buffer *)0x009B96B0;

void __cdecl construct_alphamenu_wave();
void __cdecl construct_unused_sprite_var02();
void __cdecl construct_unused_sprite_var11();
void __cdecl construct_unused_sprite_var06();
void __cdecl construct_unused_sprite_var09();
void __cdecl construct_unused_sprite_var21();
void __cdecl construct_unused_sprite_var05();
void __cdecl construct_unused_sprite_var08();
void __cdecl construct_unused_sprite_var04();
void __cdecl construct_unused_sprite_var01();
void __cdecl construct_unused_sprite_var18();
void __cdecl construct_unused_sprite_var03();
void __cdecl construct_unused_sprite_var20();
void __cdecl construct_unused_sprite_var16();
void __cdecl construct_unused_sprite_var14();
void __cdecl construct_unused_sprite_var22();
void __cdecl construct_unused_sprite_var10();
void __cdecl construct_unused_sprite_var15();
void __cdecl construct_unused_sprite_var13();
void __cdecl construct_unused_sprite_var17();
void __cdecl construct_unused_sprite_var19();
void __cdecl construct_unused_sprite_var12();
void __cdecl construct_unused_sprite_var07();
void __cdecl construct_basewin();
void __cdecl construct_basewin_wave();
void __cdecl construct_battlewin();
void __cdecl construct_councwin();
void __cdecl construct_credits_wave();
void __cdecl construct_datalink();
void __cdecl construct_designwin();
void __cdecl construct_designwin_wave();
void __cdecl construct_diplopop();
void __cdecl construct_diplowin();
void __cdecl construct_cpu_waves();
void __cdecl construct_menu_up_wave();
void __cdecl construct_menu_down_wave();
void __cdecl construct_scoot_wave();
void __cdecl construct_ok_wave();
void __cdecl construct_passover_wave();
void __cdecl construct_fx();
void __cdecl construct_ambience();
void __cdecl construct_famewin();
void __cdecl construct_pcx_parse_temp_buffer1();
void __cdecl construct_factionart();
void __cdecl construct_iface_close_x_sprites();
void __cdecl construct_iface_box_sprites1();
void __cdecl construct_iface_box_sprites2();
void __cdecl construct_iface_box_sprites3();
void __cdecl construct_iface_box_sprites4();
void __cdecl construct_iface_box_sprites5();
void __cdecl construct_iface_box_sprites6();
void __cdecl construct_iface_box_sprites7();
void __cdecl construct_iface_box_sprites8();
void __cdecl construct_iface_box_sprite_buffers();
void __cdecl construct_iface_std_popups_top_left_sprite();
void __cdecl construct_iface_std_popups_top_right_sprite();
void __cdecl construct_iface_std_popups_bot_left_sprite();
void __cdecl construct_iface_std_popups_bot_right_sprite();
void __cdecl construct_iface_std_popups_top_mid_sprite();
void __cdecl construct_iface_std_popups_bot_mid_sprite();
void __cdecl construct_iface_std_popups_mid_left_sprite();
void __cdecl construct_iface_std_popups_mid_right_sprite();
void __cdecl construct_iface_std_popups_middle_buffer();
void __cdecl construct_unused_caviardata_var1();
void __cdecl construct_ssf_caviardata();
void __cdecl construct_sdp_caviardata();
void __cdecl construct_sas_caviardata();
void __cdecl construct_scd_caviardata();
void __cdecl construct_scj_caviardata();
void __cdecl construct_sags_caviardata();
void __cdecl construct_sft_caviardata();
void __cdecl construct_vhr_caviardata1();
void __cdecl construct_sht_caviardata();
void __cdecl construct_srb_caviardata();
void __cdecl construct_asas_caviardata();
void __cdecl construct_reslaser_caviardata();
void __cdecl construct_resbolt_caviardata();
void __cdecl construct_funload_caviardata();
void __cdecl construct_tecload_caviardata();
void __cdecl construct_sp_disswave_caviardata();
void __cdecl construct_sp_marined_caviardata();
void __cdecl construct_sp_nanoo_caviardata();
void __cdecl construct_sp_soporific_caviardata();
void __cdecl construct_aa01_caviardata();
void __cdecl construct_aa_rover_caviardata();
void __cdecl construct_aa_wing_caviardata();
void __cdecl construct_acp_caviardata();
void __cdecl construct_ax_caviardata();
void __cdecl construct_aa_caviardata();
void __cdecl construct_acolpod_caviardata();
void __cdecl construct_at_caviardata();
void __cdecl construct_vta_caviardata();
void __cdecl construct_atp_caviardata();
void __cdecl construct_ssfa_caviardata();
void __cdecl construct_sfta_caviardata();
void __cdecl construct_vw00_caviardata();
void __cdecl construct_viptawl_caviardata();
void __cdecl construct_viptasgn_caviardata();
void __cdecl construct_viptapsi_caviardata();
void __cdecl construct_viptr_caviardata();
void __cdecl construct_vipta_caviardata();
void __cdecl construct_ptmod_caviardata();
void __cdecl construct_vb_caviardata();
void __cdecl construct_vbp_caviardata();
void __cdecl construct_vgmc_caviardata();
void __cdecl construct_vgmcp_caviardata();
void __cdecl construct_vlights_caviardata();
void __cdecl construct_vpt_caviardata();
void __cdecl construct_a_caviardata();
void __cdecl construct_apwall_caviardata();
void __cdecl construct_asgen_caviardata();
void __cdecl construct_apsid_caviardata();
void __cdecl construct_va01_caviardata();
void __cdecl construct_vr_caviardata();
void __cdecl construct_vhr_caviardata2();
void __cdecl construct_vrc_caviardata();
void __cdecl construct_vi_caviardata();
void __cdecl construct_vgmt_caviardata();
void __cdecl construct_vgmtp_caviardata();
void __cdecl construct_unused_caviardata_var2();
void __cdecl construct_vwntu_caviardata();
void __cdecl construct_vt_caviardata();
void __cdecl construct_drop_caviardata();
void __cdecl construct_droplet_caviardata();
void __cdecl construct_vcl_caviardata();
void __cdecl construct_unused_caviardata_var3();
void __cdecl construct_vclt00_caviardata();
void __cdecl construct_vht_vbp_caviardata();
void __cdecl construct_vhtp_caviardata();
void __cdecl construct_vhta0_caviardata();
void __cdecl construct_vhttp_caviardata();
void __cdecl construct_vhttpa0_caviardata();
void __cdecl construct_vsp_caviardata();
void __cdecl construct_vspa0_caviardata();
void __cdecl construct_vsptf_caviardata();
void __cdecl construct_vsptb_caviardata();
void __cdecl construct_vfl_caviardata();
void __cdecl construct_vgs_caviardata();
void __cdecl construct_vgsp_caviardata();
void __cdecl construct_vjtp_caviardata();
void __cdecl construct_vjt0_caviardata();
void __cdecl construct_vcu_caviardata();
void __cdecl construct_vcup_caviardata();
void __cdecl construct_vcua0_caviardata();
void __cdecl construct_vcuw_caviardata();
void __cdecl construct_vct_caviardata();
void __cdecl construct_vctp_caviardata();
void __cdecl construct_vctb_caviardata();
void __cdecl construct_vct0_caviardata();
void __cdecl construct_vwntt_caviardata();
void __cdecl construct_vwnst_caviardata();
void __cdecl construct_vwnaa_caviardata();
void __cdecl construct_vw_caviardata();
void __cdecl construct_vm_caviardata();
void __cdecl construct_vm13_caviardata();
void __cdecl construct_vpbr0_caviardata();
void __cdecl construct_unused_caviardata_var4();
void __cdecl construct_nw_caviardata();
void __cdecl construct_ni_caviardata();
void __cdecl construct_nlc_caviardata();
void __cdecl construct_unused_caviardata_var5();
void __cdecl construct_radius1_texture();
void __cdecl construct_radius2_texture();
void __cdecl construct_rocky_textures();
void __cdecl construct_ocean_textures();
void __cdecl construct_flat_arid_land_texture();
void __cdecl construct_moist_land_textures();
void __cdecl construct_rainy_land_textures();
void __cdecl construct_jungle_land_textures();
void __cdecl construct_dune_land_texture();
void __cdecl construct_sunny_mesa_textures();
void __cdecl construct_rainfall_single_tile_textures();
void __cdecl construct_road_textures();
void __cdecl construct_magtube_textures();
void __cdecl construct_river_textures();
void __cdecl construct_mount_planet_textures();
void __cdecl construct_garland_crater_textures();
void __cdecl construct_fungus_textures();
void __cdecl construct_farm_textures();
void __cdecl construct_forest_textures();
void __cdecl construct_rainfall_single_tile_texture();
void __cdecl construct_ter1_white_org_yel_tile_sprites();
void __cdecl construct_ter1_bottom_left_tile_sprites();
void __cdecl construct_ter1_unused_sprites2();
void __cdecl construct_ter1_mine_sprite();
void __cdecl construct_ter1_solar_collector_sprite();
void __cdecl construct_ter1_tidal_harness_sprite();
void __cdecl construct_ter1_mining_platform_sprite();
void __cdecl construct_ter1_tut_blank_sprite();
void __cdecl construct_ter1_kelp_farm_sprite();
void __cdecl construct_ter1_condenser_sprite();
void __cdecl construct_ter1_echelon_mirror_sprite();
void __cdecl construct_ter1_borehole_sprite();
void __cdecl construct_ter1_borehole_cluster_sprite();
void __cdecl construct_ter1_manifold_nexus_sprites();
void __cdecl construct_ter1wreck_unity_wreckage_sprites();
void __cdecl construct_ter1wreck_unity_wreckage_alt_sprites();
void __cdecl construct_fossil_field_ridge_sprites();
void __cdecl construct_ter1_unused_sprites1();
void __cdecl construct_ter1_farm_sprites();
void __cdecl construct_ter1_soil_enricher_sprites();
void __cdecl construct_ter1_sea_land_resource_sprites();
void __cdecl construct_ter1_landmark_resource_sprites();
void __cdecl construct_glow_sprites();
void __cdecl construct_ter1_unity_pod_sprites();
void __cdecl construct_ter1_monolith_sprite();
void __cdecl construct_ter1_bunker_sprite();
void __cdecl construct_ter1_airbase_sprite();
void __cdecl construct_ter1_sensor_array_sprite();
void __cdecl construct_rainfall_double_tile_sprites();
void __cdecl construct_veh_sprites();
void __cdecl construct_flags_veh_sprites();
void __cdecl construct_icons_general_sprites();
void __cdecl construct_resource_icon_sprites();
void __cdecl construct_citizen_lg_cursor_sprites();
void __cdecl construct_specialist_lg_cursor_sprites();
void __cdecl construct_citizen_sm_cursor_sprites();
void __cdecl construct_specialist_sm_cursor_sprites();
void __cdecl construct_al_citizen_lg_cursor_sprites();
void __cdecl construct_al_specialist_lg_cursor_sprites();
void __cdecl construct_al_citizen_sm_cursor_sprites();
void __cdecl construct_al_specialist_sm_cursor_sprites();
void __cdecl construct_red_alien_head_icon_sprite();
void __cdecl construct_silver_menu_icon_sprites();
void __cdecl construct_silver_checkbox_icon_sprites();
void __cdecl construct_red_male_head_icon_sprite();
void __cdecl construct_null_resource_icon_sprite();
void __cdecl construct_peace_sign_sprites();
void __cdecl construct_icon_tile_square_sprite();
void __cdecl construct_xi_boom_veh_sprites();
void __cdecl construct_xf_boom_veh_sprites();
void __cdecl construct_battle_mind_worm_sprite();
void __cdecl construct_battle_isle_deep_sprite();
void __cdecl construct_battle_locusts_chiron_sprite();
void __cdecl construct_battle_fungal_tower_sprite();
void __cdecl construct_battle_spore_launcher_sprite();
void __cdecl construct_battle_sealurk_sprite();
void __cdecl construct_tech_icon_sprites();
void __cdecl construct_facility_icon_sprites();
void __cdecl construct_secret_project_icon_sprites();
void __cdecl construct_iface_mp_combo_arrow_sprites();
void __cdecl construct_scroll_bar_arrow_icon_sprites();
void __cdecl construct_scroll_bar_small_arrow_icon_sprites();
void __cdecl construct_scroll_bar_filler_icon_sprites();
void __cdecl construct_scroll_bar_small_filler_icon_sprite();
void __cdecl construct_iface_lock_sprites();
void __cdecl construct_unused_sprites_var01();
void __cdecl construct_iface_general_windows_top_left_sprite();
void __cdecl construct_iface_general_windows_top_right_sprite();
void __cdecl construct_iface_general_windows_bot_left_sprite();
void __cdecl construct_iface_general_windows_bot_right_sprite();
void __cdecl construct_iface_general_windows_mid_left_sprite();
void __cdecl construct_iface_general_windows_mid_right_sprite();
void __cdecl construct_iface_general_windows_top_mid_sprite();
void __cdecl construct_iface_general_windows_bot_mid_sprite();
void __cdecl construct_iface_general_windows_noncap_mid_sprite();
void __cdecl construct_iface_general_windows_noncap_left_sprite();
void __cdecl construct_iface_general_windows_noncap_right_sprite();
void __cdecl construct_unused_sprite_var23();
void __cdecl construct_unused_sprite_var24();
void __cdecl construct_unused_sprite_var25();
void __cdecl construct_unused_sprite_var26();
void __cdecl construct_unused_sprite_var27();
void __cdecl construct_unused_sprite_var28();
void __cdecl construct_unused_sprite_var29();
void __cdecl construct_unused_sprite_var30();
void __cdecl construct_unused_sprite_var31();
void __cdecl construct_unused_sprite_var32();
void __cdecl construct_unused_sprite_var33();
void __cdecl construct_unused_sprite_var34();
void __cdecl construct_unused_sprite_var35();
void __cdecl construct_unused_sprite_var36();
void __cdecl construct_unused_sprite_var37();
void __cdecl construct_unused_sprite_var38();
void __cdecl construct_unused_sprite_var39();
void __cdecl construct_unused_sprite_var40();
void __cdecl construct_unused_sprite_var41();
void __cdecl construct_unused_sprite_var42();
void __cdecl construct_unused_sprite_var43();
void __cdecl construct_unused_sprite_var44();
void __cdecl construct_unused_sprite_var45();
void __cdecl construct_unused_sprite_var46();
void __cdecl construct_iface_tech_tree_arrow_sprites();
void __cdecl construct_unused_sprite_var47();
void __cdecl construct_unused_sprite_var48();
void __cdecl construct_unused_sprite_var49();
void __cdecl construct_unused_sprite_var50();
void __cdecl construct_unused_sprite_var51();
void __cdecl construct_unused_sprite_var52();
void __cdecl construct_unused_sprite_var53();
void __cdecl construct_unused_sprite_var54();
void __cdecl construct_unused_sprite_var55();
void __cdecl construct_unused_sprite_var56();
void __cdecl construct_unused_sprite_var57();
void __cdecl construct_unused_sprite_var58();
void __cdecl construct_unused_sprite_var59();
void __cdecl construct_unused_sprite_var60();
void __cdecl construct_unused_sprite_var61();
void __cdecl construct_unused_sprite_var62();
void __cdecl construct_unused_sprite_var63();
void __cdecl construct_unused_sprite_var64();
void __cdecl construct_unused_sprite_var65();
void __cdecl construct_unused_sprite_var66();
void __cdecl construct_unused_sprite_var67();
void __cdecl construct_unused_sprite_var68();
void __cdecl construct_unused_sprite_var69();
void __cdecl construct_unused_sprite_var70();
void __cdecl construct_unused_sprite_var71();
void __cdecl construct_unused_sprite_var72();
void __cdecl construct_unused_sprite_var73();
void __cdecl construct_unused_sprite_var74();
void __cdecl construct_unused_sprites_var02();
void __cdecl construct_unused_sprites_var03();
void __cdecl construct_unused_sprites_var04();
void __cdecl construct_unused_sprites_var05();
void __cdecl construct_unused_sprites_var06();
void __cdecl construct_unused_sprites_var07();
void __cdecl construct_unused_sprites_var08();
void __cdecl construct_unused_sprites_var09();
void __cdecl construct_unused_sprites_var10();
void __cdecl construct_unused_sprite_var75();
void __cdecl construct_unused_sprite_var76();
void __cdecl construct_unused_sprite_var77();
void __cdecl construct_unused_sprite_var78();
void __cdecl construct_unused_sprite_var79();
void __cdecl construct_unused_sprite_var80();
void __cdecl construct_unused_sprite_var81();
void __cdecl construct_unused_sprite_var82();
void __cdecl construct_unused_sprite_var83();
void __cdecl construct_basewin_sprites();
void __cdecl construct_infowin();
void __cdecl construct_maininterface();
void __cdecl construct_maininterface_wave();
void __cdecl construct_jackal_font();
void __cdecl construct_mapwin();
void __cdecl construct_messagewin();
void __cdecl construct_iface_green_right_arrow_sprite();
void __cdecl construct_monuwin();
void __cdecl construct_multiwin();
void __cdecl construct_multiwin_wave();
void __cdecl construct_netmsg1();
void __cdecl construct_netmsg2();
void __cdecl construct_netwin();
void __cdecl construct_newtechwin();
void __cdecl construct_newtechwin_sprites();
void __cdecl construct_pickwin();
void __cdecl construct_planwin();
void __cdecl construct_prefwin();
void __cdecl construct_prefwin_buttongroup();
void __cdecl construct_quaylewin();
void __cdecl construct_reportif();
void __cdecl construct_reportwin();
void __cdecl construct_socialwinparent();
void __cdecl construct_statuswin();
void __cdecl construct_tutwin();
void __cdecl construct_vehdraw_caviar();
void __cdecl construct_vehdraw_buffer();
void __cdecl construct_worldwin();
void __cdecl construct_wave_device();
void __cdecl construct_midi_device();
void __cdecl construct_wave_in_device();
void __cdecl construct_console();
void __cdecl construct_console_timer();
void __cdecl construct_cursor_sprites();
void __cdecl construct_netdaemon();
void __cdecl construct_fontqueue_val2();
void __cdecl construct_fontqueue_val1();
void __cdecl construct_top_menu_wave();
void __cdecl construct_fonts();
void __cdecl construct_crash_landing_wave();
void __cdecl construct_wave_general();
void __cdecl construct_palette1();
void __cdecl construct_palette2();
void __cdecl construct_multidebug();
void __cdecl construct_buffer_sprite();
void __cdecl construct_buffer();
void __cdecl construct_win_buffer();
void __cdecl construct_txtindex();
void __cdecl construct_radiobutton_sprite_1();
void __cdecl construct_radiobutton_sprite_2();
void __cdecl construct_checkbox_sprite_1();
void __cdecl construct_checkbox_sprite_2();
void __cdecl construct_filewin_sprite_1();
void __cdecl construct_filewin_sprite_2();
void __cdecl construct_filewin_sprite_3();
void __cdecl construct_stringtable();
void __cdecl construct_caviar_buffer_1();
void __cdecl construct_caviar_buffer_2();
