
int main() {
    int mode=1, selector=-1, visible=-1, animation=99; char base[80], text[160];
    ConsoleGlobal.faction_id_=0;
    BattleFightPostPromptRouteV1 route=battle_fight_normal_post_prompt_v1(
        1,2,1,2,0,-1,base,text,mode,selector,visible,animation);
    assert(route==BATTLE_FIGHT_POST_PROMPT_SOURCE_MODE_V1);
    assert(selector==0 && visible==1 && animation==0 && post_setup_calls==1);
    mode=2;
    route=battle_fight_normal_post_prompt_v1(
        1,2,1,2,0,-1,base,text,mode,selector,visible,animation);
    assert(route==BATTLE_FIGHT_POST_PROMPT_NORMAL_V1);
    return 0;
}
