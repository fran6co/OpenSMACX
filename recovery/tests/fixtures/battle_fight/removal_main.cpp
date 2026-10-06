
int main() {
    for(int i=0;i<16;++i) { Vehs[i].prev_veh_id_stack=-1; Vehs[i].next_veh_id_stack=-1;
        Vehs[i].proto_id=0; Vehs[i].dmg_incurred=10; Vehs[i].faction_id=1;
        VehPrototypes[i].plan=0; VehPrototypes[i].reactor_id=1; }
    Vehs[4].next_veh_id_stack=5; Vehs[5].prev_veh_id_stack=4;
    Vehs[5].next_veh_id_stack=6; Vehs[6].prev_veh_id_stack=5;
    int plus20=-1, minus38=0, minus30=0, minus58=0, plus18=3;
    (void)battle_fight_remove_damaged_stack_v22(4,6,0,2,&plus20,&minus38,
                                                &minus30,&minus58,0,0,&plus18);
    assert(killed_count==3 && killed[0]==4 && killed[1]==5 && killed[2]==6);
    assert(plus20==2);
    return 0;
}
