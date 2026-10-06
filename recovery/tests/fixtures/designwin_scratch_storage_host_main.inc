static void reset_host(int chassis, int weapon, int armor, int reactor) {
    Trace = HostTrace{};
    std::memset(StringTemp, 0, sizeof StringTemp);
    for (auto &p : VehPrototypes) p = VehPrototype{};
    for (auto &w : Weapon) w = WeaponRow{};
    for (auto &a : Armor) a = ArmorRow{};
    VehPrototype &p = VehPrototypes[1];
    p.icon_offset = 4;
    p.chassis_id = static_cast<uint8_t>(chassis);
    p.weapon_id = static_cast<uint8_t>(weapon);
    p.armor_id = static_cast<uint8_t>(armor);
    p.reactor_id = static_cast<uint8_t>(reactor);
    p.ability_flags = 0x80000021u;
    p.obsolete_factions = 1;
    std::strcpy(p.veh_name, "PRESET");
    Weapon[weapon].icon = 4;
    Weapon[weapon].mode = 3;
    Weapon[weapon].cost = 1;
    Armor[armor].mode = 3;
    Armor[armor].defense_rating = 1;
}

static void assert_scratch(int chassis, int weapon, int armor, int reactor, uint32_t abilities) {
    const VehPrototype &scratch = VehPrototypes[0x3f];
    assert(scratch.chassis_id == chassis);
    assert(scratch.weapon_id == weapon);
    assert(scratch.armor_id == armor);
    assert(scratch.reactor_id == reactor);
    assert(scratch.ability_flags == abilities);
    assert(scratch.icon_offset == -1);
    assert(static_cast<uint8_t>(scratch.icon_offset) == 0xff);
}

int main() {
    // Each source owner writes the actual fixed scratch row, not a parallel
    // set of host-only VehDraw variables.
    reset_host(3, 5, 7, 2);
    DesignWin publish;
    publish.protoAbilityFlags_ = 0x80000021u;
    publish.UNK3();
    assert_scratch(3, 5, 7, 2, 0x80000021u);

    // Distinct values make offset/member shifts observable. A matching row
    // must select the static preview even when Weapon.icon is absent.
    reset_host(3, 5, 7, 2);
    DesignWin matching;
    matching.protoAbilityFlags_ = 0x80000021u;
    Weapon[5].icon = -1;
    matching.draw_vehicle(0);
    assert_scratch(3, 5, 7, 2, 0x80000021u);
    assert(Trace.static_draw == 1 && Trace.construct == 0);
    assert(Trace.render == 0 && Trace.multi_render == 0);

    // Only weapon_id mismatches. Armor.defense_rating is deliberately divergent and
    // must not substitute for the original Weapon[weapon_id].icon test.
    reset_host(0, 5, 5, 2);
    DesignWin static_mismatch;
    static_mismatch.protoChassisType_ = 0;
    static_mismatch.protoWeaponType_ = 6;
    static_mismatch.protoArmorType_ = 5;
    static_mismatch.protoAbilityFlags_ = 0x80000021u;
    VehPrototypes[1].weapon_id = 5;
    Weapon[6].icon = 4;
    // Same-offset byte in the wrong table must not drive preview selection.
    Armor[5].defense_rating = -1;
    static_mismatch.draw_vehicle(0);
    assert_scratch(0, 6, 5, 2, 0x80000021u);
    assert(Trace.static_draw == 1 && Trace.construct == 0);

    // The missing Weapon icon selects construction; both Caviar draw paths
    // receive the original rectangle's unequal-edge midpoint.
    reset_host(0, 5, 5, 2);
    DesignWin constructed;
    constructed.protoChassisType_ = 0;
    constructed.protoWeaponType_ = 6;
    constructed.protoArmorType_ = 5;
    constructed.protoAbilityFlags_ = 0x80000021u;
    VehPrototypes[1].weapon_id = 5;
    Weapon[6].icon = -1;
    Armor[5].defense_rating = 4;
    constructed.draw_vehicle(0);
    assert_scratch(0, 6, 5, 2, 0x80000021u);
    assert(Trace.construct == 1 && Trace.static_draw == 0);
    assert(Trace.render == 1 && Trace.multi_render == 1);
    assert(Trace.x == 64 && Trace.y == 100);
    assert(Trace.multi_x == 64 && Trace.multi_y == 100);

    // The draw_flash source prefix is compiled from its real owner definition.
    reset_host(1, 9, 4, 3);
    DesignWin flash;
    flash.protoChassisType_ = 1;
    flash.protoWeaponType_ = 9;
    flash.protoArmorType_ = 4;
    flash.protoReactorType_ = 3;
    flash.protoAbilityFlags_ = 0xfedcba98u;
    flash.draw_flash();
    assert_scratch(1, 9, 4, 3, 0xfedcba98u);
}
