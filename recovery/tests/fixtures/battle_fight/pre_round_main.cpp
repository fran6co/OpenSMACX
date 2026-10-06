
int main() {
    Rules.move_rate_roads=3; Vehs[0].proto_id=0; Vehs[0].faction_id=1;
    Vehs[1].proto_id=64; Vehs[1].faction_id=2;
    VehPrototypes[0].chassis_id=0; VehPrototypes[0].weapon_id=0;
    VehPrototypes[0].reactor_id=2; VehPrototypes[64].chassis_id=1;
    VehPrototypes[64].weapon_id=1; VehPrototypes[64].reactor_id=3;
    Chassis[0].triad=TRIAD_SEA; Chassis[1].triad=TRIAD_SEA;
    Weapon[0].offense_rating=1; Armor[1].defense_rating=1;
    int animation=0, mode=0, report=0, direction=3, live_stack_head=10;
    BattleFightPreRoundStateV7 state=battle_fight_prepare_pre_round_v7(
        0,1,live_stack_head,11,12,8,9,mode,report,7,6,0,-1,direction,&animation);
    assert(state.early_return==0 && planet_busting_calls==0);
    VehPrototypes[0].plan=PLAN_PLANET_BUSTER;
    state=battle_fight_prepare_pre_round_v7(
        0,1,live_stack_head,11,12,8,9,mode,report,7,6,0,-1,direction,&animation);
    assert(state.early_return==1 && state.return_value==0);
    assert(planet_busting_calls==1 && (UnkBitfield1&4u)!=0);
    assert(add_goal_calls==0);
    return 0;
}
