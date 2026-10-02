#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define __cdecl
struct VehPrototype {
    char veh_name[32];
    uint32_t ability_flags;
    uint8_t chassis_id, weapon_id, armor_id, reactor_id;
    uint8_t carry_capacity, cost, plan;
    int8_t unk_1;
    uint8_t obsolete_factions;
    int8_t combat_factions, icon_offset, padding;
    uint16_t flags;
    int16_t preq_tech;
};
struct PlayerData { int prototype_history_cursor; };
VehPrototype VehPrototypes[512];
PlayerData PlayersData[8];
uint32_t LockEnableMask;

/* ACTUAL_PROTOTYPE_HISTORY */

int main() {
    unsigned char wire[0x1000];
    memset(wire, 0xA5, sizeof wire);

    // Clearing faction zero is the regression control for the native
    // base+5 / write-at-minus-5 sequence: the typed owner must write its first
    // byte at the array start, never before it.
    LockEnableMask = 0xFF;
    memset(wire, 0x31, sizeof wire);
    assert(veh_bung_copy_chunk(0, wire, sizeof wire, true));
    clear_bunglist(0);
    assert(veh_bung_copy_chunk(0, wire, sizeof wire, false));
    for (int i = 0; i < 0x200; ++i) assert(wire[i] == 0xFF);
    for (int i = 0x200; i < 0x1000; ++i) assert(wire[i] == 0x31);

    // A nonzero faction must likewise touch only its own 0x200-byte row.
    clear_bunglist(3);
    assert(veh_bung_copy_chunk(0, wire, sizeof wire, false));
    for (int i = 0; i < 0x600; ++i) {
        if (i < 0x200) assert(wire[i] == 0xFF);
        else assert(wire[i] == 0x31);
    }
    for (int i = 0x600; i < 0x800; ++i) assert(wire[i] == 0xFF);
    for (int i = 0x800; i < 0x1000; ++i) assert(wire[i] == 0x31);

    // The PlayerData +0x208C cursor selects the old slot, then wraps 63->0.
    PlayersData[3].prototype_history_cursor = 63;
    VehPrototypes[21].ability_flags = 0x12345678u;
    VehPrototypes[21].chassis_id = 2;
    VehPrototypes[21].weapon_id = 7;
    VehPrototypes[21].armor_id = 4;
    VehPrototypes[21].reactor_id = 3;
    proto_bung_put(3, 21);
    assert(PlayersData[3].prototype_history_cursor == 0);
    assert(is_bunged(3, 2, 7, 4, 0x12345678, 3));
    assert(!is_bunged(3, 2, 7, 4, 0x12345679, 3));

    // The wire helper transfers the exact full array and accepts split chunks.
    assert(veh_bung_copy_chunk(0, wire, 0x800, false));
    assert(veh_bung_copy_chunk(0x800, wire + 0x800, 0x800, false));
    assert(wire[0x600 + 63 * 8] == 0x78);
    assert(wire[0x600 + 63 * 8 + 4] == 2);
    assert(wire[0x600 + 63 * 8 + 7] == 3);
    memset(wire, 0x6D, sizeof wire);
    assert(veh_bung_copy_chunk(0, wire, sizeof wire, true));
    assert(veh_bung_copy_chunk(0, wire, sizeof wire, false));
    assert(wire[0] == 0x6D && wire[0xFFF] == 0x6D);
    assert(!veh_bung_copy_chunk(0x1000, wire, 1, false));

    puts("prototype-bung owner controls passed");
    return 0;
}
