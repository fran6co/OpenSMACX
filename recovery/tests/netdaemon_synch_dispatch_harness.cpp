#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/* SOURCE MAP DECLARATIONS */
struct Console { int faction_id_; };
Console ConsoleGlobal;

namespace NetDaemonSyncWire {
/* SOURCE MAP CAPTURE */
uint32_t current_faction();
bool copy_out(int16_t, int, int, int, size_t, size_t, void *, size_t,
              const MapRouteCapture &, uint32_t [18]);
bool copy_in(int16_t, int, int, int, size_t, size_t, const void *, size_t,
             const MapRouteCapture &, uint32_t [18]);
void commit_rules(int16_t);
bool copy_nonrule_chunk(int16_t, int, int, int, size_t, size_t,
                        unsigned char *, size_t, bool, uint32_t [18],
                        Map *, uint32_t);
}
namespace NetDaemonSyncRules {
bool copy_rule_wire_chunk(int16_t, size_t, unsigned char *, size_t, bool,
                          unsigned char *);
void commit_staged_rules(int16_t, unsigned char *);
}

static int rule_calls;
static int nonrule_calls;
static int commits;
static int last_opcode;
static size_t last_offset;
static size_t last_count;
static unsigned char *last_bytes;
static unsigned char *last_stage;
static bool last_direction;
static int last_a2, last_a3, last_a4;
static size_t last_payload;
static uint32_t *last_diplomacy;
static Map *last_tiles;
static uint32_t last_map_offset;

namespace NetDaemonSyncRules {
bool copy_rule_wire_chunk(int16_t opcode, size_t offset, unsigned char *bytes,
                          size_t count, bool direction, unsigned char *stage) {
    ++rule_calls;
    last_opcode = opcode;
    last_offset = offset;
    last_bytes = bytes;
    last_count = count;
    last_direction = direction;
    last_stage = stage;
    return true;
}
void commit_staged_rules(int16_t opcode, unsigned char *stage) {
    ++commits;
    last_opcode = opcode;
    last_stage = stage;
}
}

namespace NetDaemonSyncWire {
bool copy_nonrule_chunk(int16_t opcode, int a2, int a3, int a4,
                        size_t payload, size_t offset, unsigned char *bytes,
                        size_t count, bool direction, uint32_t diplomacy[18],
                        Map *tiles, uint32_t byte_offset) {
    ++nonrule_calls;
    last_opcode = opcode;
    last_a2 = a2; last_a3 = a3; last_a4 = a4;
    last_payload = payload; last_offset = offset; last_bytes = bytes;
    last_count = count; last_direction = direction;
    last_diplomacy = diplomacy; last_tiles = tiles;
    last_map_offset = byte_offset;
    return true;
}
}

/* SOURCE EXTRACTS */

static void reset_calls() {
    rule_calls = nonrule_calls = commits = 0;
    last_opcode = -1;
    last_offset = last_count = 0;
    last_bytes = last_stage = 0;
    last_direction = false;
    last_a2 = last_a3 = last_a4 = 0;
    last_payload = 0;
    last_diplomacy = 0;
    last_tiles = 0;
    last_map_offset = 0;
}

int main() {
    using namespace NetDaemonSyncWire;
    Map tiles[12];
    MapRouteCapture capture;
    capture.tiles = tiles;
    capture.byte_offset = 5 * 44u;
    uint32_t diplomacy[18];
    unsigned char packet[32];
    ConsoleGlobal.faction_id_ = 6;
    assert(current_faction() == 6u);

    // All fourteen rules opcodes preserve the shared persistent stage and
    // exact caller slice, in both transfer directions.
    for (int opcode = 0x80; opcode <= 0x8D; ++opcode) {
        reset_calls();
        assert(copy_out(static_cast<int16_t>(opcode), 1, 2, 3, 0x1234,
                        7, packet + 3, 9, capture, diplomacy));
        assert(rule_calls == 1 && nonrule_calls == 0);
        assert(last_opcode == opcode && last_offset == 7 && last_count == 9);
        assert(last_bytes == packet + 3 && !last_direction && last_stage != 0);
        unsigned char *persistent_stage = last_stage;

        reset_calls();
        assert(copy_in(static_cast<int16_t>(opcode), 1, 2, 3, 0x1234,
                       11, packet + 4, 8, capture, diplomacy));
        assert(rule_calls == 1 && nonrule_calls == 0);
        assert(last_opcode == opcode && last_offset == 11 && last_count == 8);
        assert(last_bytes == packet + 4 && last_direction);
        assert(last_stage == persistent_stage);

        reset_calls();
        commit_rules(static_cast<int16_t>(opcode));
        assert(commits == 1 && last_opcode == opcode);
        assert(last_stage == persistent_stage);
    }

    // Neighboring non-rule opcodes go through the owner router with all
    // captured/live arguments unchanged, and never touch the rules staging.
    reset_calls();
    assert(copy_out(0x7F, 17, 18, 19, 0x4567, 13,
                    packet + 2, 6, capture, diplomacy));
    assert(nonrule_calls == 1 && rule_calls == 0);
    assert(last_opcode == 0x7F && last_a2 == 17 && last_a3 == 18 && last_a4 == 19);
    assert(last_payload == 0x4567 && last_offset == 13);
    assert(last_bytes == packet + 2 && last_count == 6 && !last_direction);
    assert(last_diplomacy == diplomacy && last_tiles == tiles);
    assert(last_map_offset == 5 * 44u);

    reset_calls();
    assert(copy_in(0x8E, 21, 22, 23, 0x7654, 15,
                   packet + 1, 5, capture, diplomacy));
    assert(nonrule_calls == 1 && rule_calls == 0);
    assert(last_opcode == 0x8E && last_a2 == 21 && last_a3 == 22 && last_a4 == 23);
    assert(last_payload == 0x7654 && last_offset == 15);
    assert(last_bytes == packet + 1 && last_count == 5 && last_direction);
    assert(last_diplomacy == diplomacy && last_tiles == tiles);
    assert(last_map_offset == 5 * 44u);
    return 0;
}

