
int main() {
    Rules.artillery_dmg_num=20; Rules.artillery_dmg_denom=1;
    Rules.max_pct_dmg_art_vs_unit_base_bnkr=50;
    Rules.max_pct_dmg_art_vs_units_open=99; Rules.max_pct_dmg_art_vs_units_sea=100;
    Vehs[2].proto_id=2; Vehs[2].visibility=1;
    VehPrototypes[2].ability_flags=0;
    Vehs[3].proto_id=3; Vehs[3].next_veh_id_stack=4;
    Vehs[3].prev_veh_id_stack=-1; VehPrototypes[3].chassis_id=3;
    VehPrototypes[3].reactor_id=4; VehPrototypes[3].plan=PLAN_TERRAFORMING;
    Chassis[3].triad=TRIAD_LAND;
    Vehs[4].proto_id=4; Vehs[4].prev_veh_id_stack=3; Vehs[4].next_veh_id_stack=-1;
    VehPrototypes[4].chassis_id=4; VehPrototypes[4].reactor_id=4;
    VehPrototypes[4].plan=PLAN_TERRAFORMING; Chassis[4].triad=TRIAD_AIR;
    target_stack_root=3; TestTile.bit=0; GamePreferences=4;
    assert(battle_fight_source_mode(2,3,2,1,6,8,4,5,-1)==0);
    assert(compute_calls==1 && skip_id==2 && ConsoleStatusWin.field_15D4_==1 && ConsoleStatusWin.stores==1 && status_flushes==1 && ConsoleStatusWin.draws==2);
    TestTile.bit=0x40000;
    assert(battle_fight_source_mode(2,3,2,1,6,8,4,5,-1)==0);
    assert(compute_calls==3); // target 0x40000 admits the air member too
    assert(fixups==2 && ConsoleStatusWin.stores==3 && status_flushes==3);
    return 0;
}
