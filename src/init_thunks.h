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
 * catalogued ??__F teardown with the game CRT's atexit - through the
 * GameAtexit seam, because the game CRT's exit list must own the registration
 * so teardown order stays inside its LIFO walk. The registered value is the
 * ORIGINAL ??__F address, which the hybrid redirects to the recovered
 * destroy_* twin at run time. Globals shared with the teardown side stay
 * declared in atexit_thunks.h; only the ones the teardown side does not bind
 * appear here.
 */

typedef void(__cdecl func_atexit_callback)();
typedef int(__cdecl func_game_atexit)(func_atexit_callback *callback);
extern func_game_atexit *GameAtexit;   // 0x00645398

#include "vector_teardown.h"
extern func_thiscall_teardown *BufferElementCtor;
extern func_thiscall_teardown *CaviarDataElementCtor;
extern func_thiscall_teardown *FontElementCtor;
extern func_thiscall_teardown *SpriteElementCtor;
extern func_thiscall_teardown *TextIndexElementCtor;
extern func_thiscall_teardown *TextureElementCtor;
extern func_thiscall_teardown *WaveElementCtor;

// Per-kind scalar construction seams, defaulting to
// source-owned trampolines over the recovered
// constructors. Rebindable because those constructors
// reach their own deeper seams, whose defaults are
// unmapped outside the game process.
extern func_thiscall_teardown *AmbienceInitCtor;
extern func_thiscall_teardown *BufferInitCtor;
extern func_thiscall_teardown *ButtonGroupInitCtor;
extern func_thiscall_teardown *CaviarDataInitCtor;
extern func_thiscall_teardown *CaviarInitCtor;
extern func_thiscall_teardown *FXInitCtor;
extern func_thiscall_teardown *FontInitCtor;
extern func_thiscall_teardown *SpriteInitCtor;
extern func_thiscall_teardown *StringsInitCtor;
extern func_thiscall_teardown *TextureInitCtor;
extern func_thiscall_teardown *TimeInitCtor;
extern func_thiscall_teardown *WaveDeviceInitCtor;
extern func_thiscall_teardown *WaveInitCtor;

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"
#endif
typedef void(__thiscall func_opaque_ctor_i)(void *object, int a0);
typedef void(__thiscall func_opaque_ctor_iii)(void *object, int a0, int a1,
                                              int a2);
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

// Opaque-storage construction seams. These globals' classes
// are not modelled in source, so the thunk hands the storage
// address straight to the original constructor through a seam
// rather than naming a type. Seams are deduped on the ADDRESS
// they default to; one already bound at the same address
// elsewhere is reused, never redefined here.
extern func_thiscall_teardown *BaseWinCtorTarget;   // 0x00408490
extern func_thiscall_teardown *BattleWinCtorTarget;   // 0x00422EE0
extern func_thiscall_teardown *CouncWinCtorTarget;   // 0x00428620
extern func_thiscall_teardown *DatalinkCtorTarget;   // 0x00428FC0
extern func_thiscall_teardown *DesignWinCtorTarget;   // 0x00434480
extern func_thiscall_teardown *DiploPopCtorTarget;   // 0x0043EFF0
extern func_thiscall_teardown *DiploWinCtorTarget;   // 0x00444FC0
extern func_thiscall_teardown *FameWinCtorTarget;   // 0x0044B200
extern func_thiscall_teardown *FactionArtCtorTarget;   // 0x00455F10
extern func_thiscall_teardown *InfoWinCtorTarget;   // 0x00459500
extern func_thiscall_teardown *MainInterfaceCtorTarget;   // 0x0045EF10
extern func_opaque_ctor_i *MapWinCtorTarget;   // 0x004626E0
extern func_thiscall_teardown *MessageWinCtorTarget;   // 0x00472190
extern func_thiscall_teardown *MonuWinCtorTarget;   // 0x00477C80
extern func_thiscall_teardown *MultiWinCtorTarget;   // 0x0047A590
extern func_opaque_ctor_iii *NetMsgCtorTarget;   // 0x0047ACF0
extern func_thiscall_teardown *NetWinCtorTarget;   // 0x00481C50
extern func_thiscall_teardown *NewTechWinCtorTarget;   // 0x004849D0
extern func_thiscall_teardown *PickWinCtorTarget;   // 0x0048AC10
extern func_opaque_ctor_i *PlanWinCtorTarget;   // 0x0048BCD0
extern func_thiscall_teardown *PrefWinCtorTarget;   // 0x004921E0
extern func_thiscall_teardown *QuayleWinCtorTarget;   // 0x00496810
extern func_thiscall_teardown *ReportIfCtorTarget;   // 0x004AD170
extern func_thiscall_teardown *ReportWinCtorTarget;   // 0x004AD6B0
extern func_thiscall_teardown *SocialWinCtorTarget;   // 0x004AE9E0
extern func_thiscall_teardown *StatusWinCtorTarget;   // 0x004BA1A0
extern func_thiscall_teardown *TutWinCtorTarget;   // 0x004BA6B0
extern func_thiscall_teardown *WorldWinCtorTarget;   // 0x004C4BF0
extern func_thiscall_teardown *Midi_DeviceCtorTarget;   // 0x004C5740
extern func_thiscall_teardown *Wave_In_DeviceCtorTarget;   // 0x004C5940
extern func_opaque_ctor_i *ConsoleCtorTarget;   // 0x0050F460
extern func_thiscall_teardown *NetDaemonCtorTarget;   // 0x005389F0
extern func_thiscall_teardown *FontQueueCtorTarget;   // 0x00559290
extern func_thiscall_teardown *MultiDebugCtorTarget;   // 0x005C97F0
extern func_thiscall_teardown *PaletteCtorTarget;   // 0x005FE2A0

extern Sprite *g_NEWTECHWIN_SPRITES;
extern Caviar *g_VEHDRAW_CAVIAR;
extern Wave_Device *g_WAVE_DEVICE;
extern Time *g_CONSOLE_TIMER;
extern Sprite *g_BUFFER_SPRITE;
extern Buffer *g_WIN_BUFFER;
extern Sprite *g_RADIOBUTTON_SPRITE_1;
extern Sprite *g_RADIOBUTTON_SPRITE_2;
extern Sprite *g_CHECKBOX_SPRITE_1;
extern Sprite *g_CHECKBOX_SPRITE_2;
extern Sprite *g_FILEWIN_SPRITE_1;
extern Sprite *g_FILEWIN_SPRITE_2;
extern Sprite *g_FILEWIN_SPRITE_3;
extern Buffer *g_CAVIAR_BUFFER_1;
extern Buffer *g_CAVIAR_BUFFER_2;

DLLEXPORT void __cdecl construct_alphamenu_wave();
DLLEXPORT void __cdecl construct_unused_sprite_var02();
DLLEXPORT void __cdecl construct_unused_sprite_var11();
DLLEXPORT void __cdecl construct_unused_sprite_var06();
DLLEXPORT void __cdecl construct_unused_sprite_var09();
DLLEXPORT void __cdecl construct_unused_sprite_var21();
DLLEXPORT void __cdecl construct_unused_sprite_var05();
DLLEXPORT void __cdecl construct_unused_sprite_var08();
DLLEXPORT void __cdecl construct_unused_sprite_var04();
DLLEXPORT void __cdecl construct_unused_sprite_var01();
DLLEXPORT void __cdecl construct_unused_sprite_var18();
DLLEXPORT void __cdecl construct_unused_sprite_var03();
DLLEXPORT void __cdecl construct_unused_sprite_var20();
DLLEXPORT void __cdecl construct_unused_sprite_var16();
DLLEXPORT void __cdecl construct_unused_sprite_var14();
DLLEXPORT void __cdecl construct_unused_sprite_var22();
DLLEXPORT void __cdecl construct_unused_sprite_var10();
DLLEXPORT void __cdecl construct_unused_sprite_var15();
DLLEXPORT void __cdecl construct_unused_sprite_var13();
DLLEXPORT void __cdecl construct_unused_sprite_var17();
DLLEXPORT void __cdecl construct_unused_sprite_var19();
DLLEXPORT void __cdecl construct_unused_sprite_var12();
DLLEXPORT void __cdecl construct_unused_sprite_var07();
DLLEXPORT void __cdecl construct_basewin();
DLLEXPORT void __cdecl construct_basewin_wave();
DLLEXPORT void __cdecl construct_battlewin();
DLLEXPORT void __cdecl construct_councwin();
DLLEXPORT void __cdecl construct_credits_wave();
DLLEXPORT void __cdecl construct_datalink();
DLLEXPORT void __cdecl construct_designwin();
DLLEXPORT void __cdecl construct_designwin_wave();
DLLEXPORT void __cdecl construct_diplopop();
DLLEXPORT void __cdecl construct_diplowin();
DLLEXPORT void __cdecl construct_cpu_waves();
DLLEXPORT void __cdecl construct_menu_up_wave();
DLLEXPORT void __cdecl construct_menu_down_wave();
DLLEXPORT void __cdecl construct_scoot_wave();
DLLEXPORT void __cdecl construct_ok_wave();
DLLEXPORT void __cdecl construct_passover_wave();
DLLEXPORT void __cdecl construct_fx();
DLLEXPORT void __cdecl construct_ambience();
DLLEXPORT void __cdecl construct_famewin();
DLLEXPORT void __cdecl construct_pcx_parse_temp_buffer1();
DLLEXPORT void __cdecl construct_factionart();
DLLEXPORT void __cdecl construct_iface_close_x_sprites();
DLLEXPORT void __cdecl construct_iface_box_sprites1();
DLLEXPORT void __cdecl construct_iface_box_sprites2();
DLLEXPORT void __cdecl construct_iface_box_sprites3();
DLLEXPORT void __cdecl construct_iface_box_sprites4();
DLLEXPORT void __cdecl construct_iface_box_sprites5();
DLLEXPORT void __cdecl construct_iface_box_sprites6();
DLLEXPORT void __cdecl construct_iface_box_sprites7();
DLLEXPORT void __cdecl construct_iface_box_sprites8();
DLLEXPORT void __cdecl construct_iface_box_sprite_buffers();
DLLEXPORT void __cdecl construct_iface_std_popups_top_left_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_top_right_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_bot_left_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_bot_right_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_top_mid_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_bot_mid_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_mid_left_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_mid_right_sprite();
DLLEXPORT void __cdecl construct_iface_std_popups_middle_buffer();
DLLEXPORT void __cdecl construct_unused_caviardata_var1();
DLLEXPORT void __cdecl construct_ssf_caviardata();
DLLEXPORT void __cdecl construct_sdp_caviardata();
DLLEXPORT void __cdecl construct_sas_caviardata();
DLLEXPORT void __cdecl construct_scd_caviardata();
DLLEXPORT void __cdecl construct_scj_caviardata();
DLLEXPORT void __cdecl construct_sags_caviardata();
DLLEXPORT void __cdecl construct_sft_caviardata();
DLLEXPORT void __cdecl construct_vhr_caviardata1();
DLLEXPORT void __cdecl construct_sht_caviardata();
DLLEXPORT void __cdecl construct_srb_caviardata();
DLLEXPORT void __cdecl construct_asas_caviardata();
DLLEXPORT void __cdecl construct_reslaser_caviardata();
DLLEXPORT void __cdecl construct_resbolt_caviardata();
DLLEXPORT void __cdecl construct_funload_caviardata();
DLLEXPORT void __cdecl construct_tecload_caviardata();
DLLEXPORT void __cdecl construct_sp_disswave_caviardata();
DLLEXPORT void __cdecl construct_sp_marined_caviardata();
DLLEXPORT void __cdecl construct_sp_nanoo_caviardata();
DLLEXPORT void __cdecl construct_sp_soporific_caviardata();
DLLEXPORT void __cdecl construct_aa01_caviardata();
DLLEXPORT void __cdecl construct_aa_rover_caviardata();
DLLEXPORT void __cdecl construct_aa_wing_caviardata();
DLLEXPORT void __cdecl construct_acp_caviardata();
DLLEXPORT void __cdecl construct_ax_caviardata();
DLLEXPORT void __cdecl construct_aa_caviardata();
DLLEXPORT void __cdecl construct_acolpod_caviardata();
DLLEXPORT void __cdecl construct_at_caviardata();
DLLEXPORT void __cdecl construct_vta_caviardata();
DLLEXPORT void __cdecl construct_atp_caviardata();
DLLEXPORT void __cdecl construct_ssfa_caviardata();
DLLEXPORT void __cdecl construct_sfta_caviardata();
DLLEXPORT void __cdecl construct_vw00_caviardata();
DLLEXPORT void __cdecl construct_viptawl_caviardata();
DLLEXPORT void __cdecl construct_viptasgn_caviardata();
DLLEXPORT void __cdecl construct_viptapsi_caviardata();
DLLEXPORT void __cdecl construct_viptr_caviardata();
DLLEXPORT void __cdecl construct_vipta_caviardata();
DLLEXPORT void __cdecl construct_ptmod_caviardata();
DLLEXPORT void __cdecl construct_vb_caviardata();
DLLEXPORT void __cdecl construct_vbp_caviardata();
DLLEXPORT void __cdecl construct_vgmc_caviardata();
DLLEXPORT void __cdecl construct_vgmcp_caviardata();
DLLEXPORT void __cdecl construct_vlights_caviardata();
DLLEXPORT void __cdecl construct_vpt_caviardata();
DLLEXPORT void __cdecl construct_a_caviardata();
DLLEXPORT void __cdecl construct_apwall_caviardata();
DLLEXPORT void __cdecl construct_asgen_caviardata();
DLLEXPORT void __cdecl construct_apsid_caviardata();
DLLEXPORT void __cdecl construct_va01_caviardata();
DLLEXPORT void __cdecl construct_vr_caviardata();
DLLEXPORT void __cdecl construct_vhr_caviardata2();
DLLEXPORT void __cdecl construct_vrc_caviardata();
DLLEXPORT void __cdecl construct_vi_caviardata();
DLLEXPORT void __cdecl construct_vgmt_caviardata();
DLLEXPORT void __cdecl construct_vgmtp_caviardata();
DLLEXPORT void __cdecl construct_unused_caviardata_var2();
DLLEXPORT void __cdecl construct_vwntu_caviardata();
DLLEXPORT void __cdecl construct_vt_caviardata();
DLLEXPORT void __cdecl construct_drop_caviardata();
DLLEXPORT void __cdecl construct_droplet_caviardata();
DLLEXPORT void __cdecl construct_vcl_caviardata();
DLLEXPORT void __cdecl construct_unused_caviardata_var3();
DLLEXPORT void __cdecl construct_vclt00_caviardata();
DLLEXPORT void __cdecl construct_vht_vbp_caviardata();
DLLEXPORT void __cdecl construct_vhtp_caviardata();
DLLEXPORT void __cdecl construct_vhta0_caviardata();
DLLEXPORT void __cdecl construct_vhttp_caviardata();
DLLEXPORT void __cdecl construct_vhttpa0_caviardata();
DLLEXPORT void __cdecl construct_vsp_caviardata();
DLLEXPORT void __cdecl construct_vspa0_caviardata();
DLLEXPORT void __cdecl construct_vsptf_caviardata();
DLLEXPORT void __cdecl construct_vsptb_caviardata();
DLLEXPORT void __cdecl construct_vfl_caviardata();
DLLEXPORT void __cdecl construct_vgs_caviardata();
DLLEXPORT void __cdecl construct_vgsp_caviardata();
DLLEXPORT void __cdecl construct_vjtp_caviardata();
DLLEXPORT void __cdecl construct_vjt0_caviardata();
DLLEXPORT void __cdecl construct_vcu_caviardata();
DLLEXPORT void __cdecl construct_vcup_caviardata();
DLLEXPORT void __cdecl construct_vcua0_caviardata();
DLLEXPORT void __cdecl construct_vcuw_caviardata();
DLLEXPORT void __cdecl construct_vct_caviardata();
DLLEXPORT void __cdecl construct_vctp_caviardata();
DLLEXPORT void __cdecl construct_vctb_caviardata();
DLLEXPORT void __cdecl construct_vct0_caviardata();
DLLEXPORT void __cdecl construct_vwntt_caviardata();
DLLEXPORT void __cdecl construct_vwnst_caviardata();
DLLEXPORT void __cdecl construct_vwnaa_caviardata();
DLLEXPORT void __cdecl construct_vw_caviardata();
DLLEXPORT void __cdecl construct_vm_caviardata();
DLLEXPORT void __cdecl construct_vm13_caviardata();
DLLEXPORT void __cdecl construct_vpbr0_caviardata();
DLLEXPORT void __cdecl construct_unused_caviardata_var4();
DLLEXPORT void __cdecl construct_nw_caviardata();
DLLEXPORT void __cdecl construct_ni_caviardata();
DLLEXPORT void __cdecl construct_nlc_caviardata();
DLLEXPORT void __cdecl construct_unused_caviardata_var5();
DLLEXPORT void __cdecl construct_radius1_texture();
DLLEXPORT void __cdecl construct_radius2_texture();
DLLEXPORT void __cdecl construct_rocky_textures();
DLLEXPORT void __cdecl construct_ocean_textures();
DLLEXPORT void __cdecl construct_flat_arid_land_texture();
DLLEXPORT void __cdecl construct_moist_land_textures();
DLLEXPORT void __cdecl construct_rainy_land_textures();
DLLEXPORT void __cdecl construct_jungle_land_textures();
DLLEXPORT void __cdecl construct_dune_land_texture();
DLLEXPORT void __cdecl construct_sunny_mesa_textures();
DLLEXPORT void __cdecl construct_rainfall_single_tile_textures();
DLLEXPORT void __cdecl construct_road_textures();
DLLEXPORT void __cdecl construct_magtube_textures();
DLLEXPORT void __cdecl construct_river_textures();
DLLEXPORT void __cdecl construct_mount_planet_textures();
DLLEXPORT void __cdecl construct_garland_crater_textures();
DLLEXPORT void __cdecl construct_fungus_textures();
DLLEXPORT void __cdecl construct_farm_textures();
DLLEXPORT void __cdecl construct_forest_textures();
DLLEXPORT void __cdecl construct_rainfall_single_tile_texture();
DLLEXPORT void __cdecl construct_ter1_white_org_yel_tile_sprites();
DLLEXPORT void __cdecl construct_ter1_bottom_left_tile_sprites();
DLLEXPORT void __cdecl construct_ter1_unused_sprites2();
DLLEXPORT void __cdecl construct_ter1_mine_sprite();
DLLEXPORT void __cdecl construct_ter1_solar_collector_sprite();
DLLEXPORT void __cdecl construct_ter1_tidal_harness_sprite();
DLLEXPORT void __cdecl construct_ter1_mining_platform_sprite();
DLLEXPORT void __cdecl construct_ter1_tut_blank_sprite();
DLLEXPORT void __cdecl construct_ter1_kelp_farm_sprite();
DLLEXPORT void __cdecl construct_ter1_condenser_sprite();
DLLEXPORT void __cdecl construct_ter1_echelon_mirror_sprite();
DLLEXPORT void __cdecl construct_ter1_borehole_sprite();
DLLEXPORT void __cdecl construct_ter1_borehole_cluster_sprite();
DLLEXPORT void __cdecl construct_ter1_manifold_nexus_sprites();
DLLEXPORT void __cdecl construct_ter1wreck_unity_wreckage_sprites();
DLLEXPORT void __cdecl construct_ter1wreck_unity_wreckage_alt_sprites();
DLLEXPORT void __cdecl construct_fossil_field_ridge_sprites();
DLLEXPORT void __cdecl construct_ter1_unused_sprites1();
DLLEXPORT void __cdecl construct_ter1_farm_sprites();
DLLEXPORT void __cdecl construct_ter1_soil_enricher_sprites();
DLLEXPORT void __cdecl construct_ter1_sea_land_resource_sprites();
DLLEXPORT void __cdecl construct_ter1_landmark_resource_sprites();
DLLEXPORT void __cdecl construct_glow_sprites();
DLLEXPORT void __cdecl construct_ter1_unity_pod_sprites();
DLLEXPORT void __cdecl construct_ter1_monolith_sprite();
DLLEXPORT void __cdecl construct_ter1_bunker_sprite();
DLLEXPORT void __cdecl construct_ter1_airbase_sprite();
DLLEXPORT void __cdecl construct_ter1_sensor_array_sprite();
DLLEXPORT void __cdecl construct_rainfall_double_tile_sprites();
DLLEXPORT void __cdecl construct_veh_sprites();
DLLEXPORT void __cdecl construct_flags_veh_sprites();
DLLEXPORT void __cdecl construct_icons_general_sprites();
DLLEXPORT void __cdecl construct_resource_icon_sprites();
DLLEXPORT void __cdecl construct_citizen_lg_cursor_sprites();
DLLEXPORT void __cdecl construct_specialist_lg_cursor_sprites();
DLLEXPORT void __cdecl construct_citizen_sm_cursor_sprites();
DLLEXPORT void __cdecl construct_specialist_sm_cursor_sprites();
DLLEXPORT void __cdecl construct_al_citizen_lg_cursor_sprites();
DLLEXPORT void __cdecl construct_al_specialist_lg_cursor_sprites();
DLLEXPORT void __cdecl construct_al_citizen_sm_cursor_sprites();
DLLEXPORT void __cdecl construct_al_specialist_sm_cursor_sprites();
DLLEXPORT void __cdecl construct_red_alien_head_icon_sprite();
DLLEXPORT void __cdecl construct_silver_menu_icon_sprites();
DLLEXPORT void __cdecl construct_silver_checkbox_icon_sprites();
DLLEXPORT void __cdecl construct_red_male_head_icon_sprite();
DLLEXPORT void __cdecl construct_null_resource_icon_sprite();
DLLEXPORT void __cdecl construct_peace_sign_sprites();
DLLEXPORT void __cdecl construct_icon_tile_square_sprite();
DLLEXPORT void __cdecl construct_xi_boom_veh_sprites();
DLLEXPORT void __cdecl construct_xf_boom_veh_sprites();
DLLEXPORT void __cdecl construct_battle_mind_worm_sprite();
DLLEXPORT void __cdecl construct_battle_isle_deep_sprite();
DLLEXPORT void __cdecl construct_battle_locusts_chiron_sprite();
DLLEXPORT void __cdecl construct_battle_fungal_tower_sprite();
DLLEXPORT void __cdecl construct_battle_spore_launcher_sprite();
DLLEXPORT void __cdecl construct_battle_sealurk_sprite();
DLLEXPORT void __cdecl construct_tech_icon_sprites();
DLLEXPORT void __cdecl construct_facility_icon_sprites();
DLLEXPORT void __cdecl construct_secret_project_icon_sprites();
DLLEXPORT void __cdecl construct_iface_mp_combo_arrow_sprites();
DLLEXPORT void __cdecl construct_scroll_bar_arrow_icon_sprites();
DLLEXPORT void __cdecl construct_scroll_bar_small_arrow_icon_sprites();
DLLEXPORT void __cdecl construct_scroll_bar_filler_icon_sprites();
DLLEXPORT void __cdecl construct_scroll_bar_small_filler_icon_sprite();
DLLEXPORT void __cdecl construct_iface_lock_sprites();
DLLEXPORT void __cdecl construct_unused_sprites_var01();
DLLEXPORT void __cdecl construct_iface_general_windows_top_left_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_top_right_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_bot_left_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_bot_right_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_mid_left_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_mid_right_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_top_mid_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_bot_mid_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_noncap_mid_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_noncap_left_sprite();
DLLEXPORT void __cdecl construct_iface_general_windows_noncap_right_sprite();
DLLEXPORT void __cdecl construct_unused_sprite_var23();
DLLEXPORT void __cdecl construct_unused_sprite_var24();
DLLEXPORT void __cdecl construct_unused_sprite_var25();
DLLEXPORT void __cdecl construct_unused_sprite_var26();
DLLEXPORT void __cdecl construct_unused_sprite_var27();
DLLEXPORT void __cdecl construct_unused_sprite_var28();
DLLEXPORT void __cdecl construct_unused_sprite_var29();
DLLEXPORT void __cdecl construct_unused_sprite_var30();
DLLEXPORT void __cdecl construct_unused_sprite_var31();
DLLEXPORT void __cdecl construct_unused_sprite_var32();
DLLEXPORT void __cdecl construct_unused_sprite_var33();
DLLEXPORT void __cdecl construct_unused_sprite_var34();
DLLEXPORT void __cdecl construct_unused_sprite_var35();
DLLEXPORT void __cdecl construct_unused_sprite_var36();
DLLEXPORT void __cdecl construct_unused_sprite_var37();
DLLEXPORT void __cdecl construct_unused_sprite_var38();
DLLEXPORT void __cdecl construct_unused_sprite_var39();
DLLEXPORT void __cdecl construct_unused_sprite_var40();
DLLEXPORT void __cdecl construct_unused_sprite_var41();
DLLEXPORT void __cdecl construct_unused_sprite_var42();
DLLEXPORT void __cdecl construct_unused_sprite_var43();
DLLEXPORT void __cdecl construct_unused_sprite_var44();
DLLEXPORT void __cdecl construct_unused_sprite_var45();
DLLEXPORT void __cdecl construct_unused_sprite_var46();
DLLEXPORT void __cdecl construct_iface_tech_tree_arrow_sprites();
DLLEXPORT void __cdecl construct_unused_sprite_var47();
DLLEXPORT void __cdecl construct_unused_sprite_var48();
DLLEXPORT void __cdecl construct_unused_sprite_var49();
DLLEXPORT void __cdecl construct_unused_sprite_var50();
DLLEXPORT void __cdecl construct_unused_sprite_var51();
DLLEXPORT void __cdecl construct_unused_sprite_var52();
DLLEXPORT void __cdecl construct_unused_sprite_var53();
DLLEXPORT void __cdecl construct_unused_sprite_var54();
DLLEXPORT void __cdecl construct_unused_sprite_var55();
DLLEXPORT void __cdecl construct_unused_sprite_var56();
DLLEXPORT void __cdecl construct_unused_sprite_var57();
DLLEXPORT void __cdecl construct_unused_sprite_var58();
DLLEXPORT void __cdecl construct_unused_sprite_var59();
DLLEXPORT void __cdecl construct_unused_sprite_var60();
DLLEXPORT void __cdecl construct_unused_sprite_var61();
DLLEXPORT void __cdecl construct_unused_sprite_var62();
DLLEXPORT void __cdecl construct_unused_sprite_var63();
DLLEXPORT void __cdecl construct_unused_sprite_var64();
DLLEXPORT void __cdecl construct_unused_sprite_var65();
DLLEXPORT void __cdecl construct_unused_sprite_var66();
DLLEXPORT void __cdecl construct_unused_sprite_var67();
DLLEXPORT void __cdecl construct_unused_sprite_var68();
DLLEXPORT void __cdecl construct_unused_sprite_var69();
DLLEXPORT void __cdecl construct_unused_sprite_var70();
DLLEXPORT void __cdecl construct_unused_sprite_var71();
DLLEXPORT void __cdecl construct_unused_sprite_var72();
DLLEXPORT void __cdecl construct_unused_sprite_var73();
DLLEXPORT void __cdecl construct_unused_sprite_var74();
DLLEXPORT void __cdecl construct_unused_sprites_var02();
DLLEXPORT void __cdecl construct_unused_sprites_var03();
DLLEXPORT void __cdecl construct_unused_sprites_var04();
DLLEXPORT void __cdecl construct_unused_sprites_var05();
DLLEXPORT void __cdecl construct_unused_sprites_var06();
DLLEXPORT void __cdecl construct_unused_sprites_var07();
DLLEXPORT void __cdecl construct_unused_sprites_var08();
DLLEXPORT void __cdecl construct_unused_sprites_var09();
DLLEXPORT void __cdecl construct_unused_sprites_var10();
DLLEXPORT void __cdecl construct_unused_sprite_var75();
DLLEXPORT void __cdecl construct_unused_sprite_var76();
DLLEXPORT void __cdecl construct_unused_sprite_var77();
DLLEXPORT void __cdecl construct_unused_sprite_var78();
DLLEXPORT void __cdecl construct_unused_sprite_var79();
DLLEXPORT void __cdecl construct_unused_sprite_var80();
DLLEXPORT void __cdecl construct_unused_sprite_var81();
DLLEXPORT void __cdecl construct_unused_sprite_var82();
DLLEXPORT void __cdecl construct_unused_sprite_var83();
DLLEXPORT void __cdecl construct_basewin_sprites();
DLLEXPORT void __cdecl construct_infowin();
DLLEXPORT void __cdecl construct_maininterface();
DLLEXPORT void __cdecl construct_maininterface_wave();
DLLEXPORT void __cdecl construct_jackal_font();
DLLEXPORT void __cdecl construct_mapwin();
DLLEXPORT void __cdecl construct_messagewin();
DLLEXPORT void __cdecl construct_iface_green_right_arrow_sprite();
DLLEXPORT void __cdecl construct_monuwin();
DLLEXPORT void __cdecl construct_multiwin();
DLLEXPORT void __cdecl construct_multiwin_wave();
DLLEXPORT void __cdecl construct_netmsg1();
DLLEXPORT void __cdecl construct_netmsg2();
DLLEXPORT void __cdecl construct_netwin();
DLLEXPORT void __cdecl construct_newtechwin();
DLLEXPORT void __cdecl construct_newtechwin_sprites();
DLLEXPORT void __cdecl construct_pickwin();
DLLEXPORT void __cdecl construct_planwin();
DLLEXPORT void __cdecl construct_prefwin();
DLLEXPORT void __cdecl construct_prefwin_buttongroup();
DLLEXPORT void __cdecl construct_quaylewin();
DLLEXPORT void __cdecl construct_reportif();
DLLEXPORT void __cdecl construct_reportwin();
DLLEXPORT void __cdecl construct_socialwinparent();
DLLEXPORT void __cdecl construct_statuswin();
DLLEXPORT void __cdecl construct_tutwin();
DLLEXPORT void __cdecl construct_vehdraw_caviar();
DLLEXPORT void __cdecl construct_vehdraw_buffer();
DLLEXPORT void __cdecl construct_worldwin();
DLLEXPORT void __cdecl construct_wave_device();
DLLEXPORT void __cdecl construct_midi_device();
DLLEXPORT void __cdecl construct_wave_in_device();
DLLEXPORT void __cdecl construct_console();
DLLEXPORT void __cdecl construct_console_timer();
DLLEXPORT void __cdecl construct_cursor_sprites();
DLLEXPORT void __cdecl construct_netdaemon();
DLLEXPORT void __cdecl construct_fontqueue_val2();
DLLEXPORT void __cdecl construct_fontqueue_val1();
DLLEXPORT void __cdecl construct_top_menu_wave();
DLLEXPORT void __cdecl construct_fonts();
DLLEXPORT void __cdecl construct_crash_landing_wave();
DLLEXPORT void __cdecl construct_wave_general();
DLLEXPORT void __cdecl construct_palette1();
DLLEXPORT void __cdecl construct_palette2();
DLLEXPORT void __cdecl construct_multidebug();
DLLEXPORT void __cdecl construct_buffer_sprite();
DLLEXPORT void __cdecl construct_buffer();
DLLEXPORT void __cdecl construct_win_buffer();
DLLEXPORT void __cdecl construct_txtindex();
DLLEXPORT void __cdecl construct_radiobutton_sprite_1();
DLLEXPORT void __cdecl construct_radiobutton_sprite_2();
DLLEXPORT void __cdecl construct_checkbox_sprite_1();
DLLEXPORT void __cdecl construct_checkbox_sprite_2();
DLLEXPORT void __cdecl construct_filewin_sprite_1();
DLLEXPORT void __cdecl construct_filewin_sprite_2();
DLLEXPORT void __cdecl construct_filewin_sprite_3();
DLLEXPORT void __cdecl construct_stringtable();
DLLEXPORT void __cdecl construct_caviar_buffer_1();
DLLEXPORT void __cdecl construct_caviar_buffer_2();
