// Private host mutation-footprint harness. The test driver prepends the
// current game.h struct declarations and game.cpp reset bodies verbatim.
// This file contains only backing storage and assertions, not copied bodies.

static int fail(const char *message) {
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

static int verify_offer_pairs() {
    OfferResetCell before[8][8];
    OfferResetCell expected[8][8];
    for (int a = 0; a < 8; ++a) {
        for (int b = 0; b < 8; ++b) {
            int *words = reinterpret_cast<int *>(&OfferResetCells[a][b]);
            for (int word = 0; word < 36; ++word) {
                words[word] = 100000 + a * 1000 + b * 40 + word;
            }
        }
    }

    for (int a = 0; a < 8; ++a) {
        for (int b = 0; b < 8; ++b) {
            memcpy(before, OfferResetCells, sizeof before);
            memcpy(expected, before, sizeof expected);
            expected[a][b].count_[0] = 0;
            expected[a][b].count_[1] = 0;
            expected[b][a].count_[0] = 0;
            expected[b][a].count_[1] = 0;
            expected[a][b].state_[0] = 0;
            expected[a][b].state_[1] = 0;
            expected[b][a].state_[0] = 0;
            expected[b][a].state_[1] = 0;
            clear_offer_pair(a, b);
            if (memcmp(expected, OfferResetCells, sizeof expected) != 0) {
                return fail("offer pair mutation differs from exact two-cell footprint");
            }
        }
    }
    return 0;
}

static int verify_tamper_registry() {
    TamperRegistryCursor = 0x13572468;
    TamperBacking.trailing_guard = 0x24681357;
    for (int index = 0; index < 50; ++index) {
        TamperRecord &record = TamperRegistryRecords[index];
        record.field_0_ = 1000 + index * 6;
        record.field_4_ = 1001 + index * 6;
        record.field_8_ = 1002 + index * 6;
        record.field_C_ = 1003 + index * 6;
        record.count_ = 1004 + index * 6;
        record.field_14_ = 1005 + index * 6;
    }

    clear_tamper_registry();
    if (TamperRegistryCursor != 0) {
        return fail("tamper registry cursor was not reset");
    }
    if (TamperBacking.trailing_guard != 0x24681357) {
        return fail("tamper reset wrote past the 50-record source array");
    }
    for (int index = 0; index < 50; ++index) {
        const TamperRecord &record = TamperRegistryRecords[index];
        if (record.field_0_ != 0 || record.field_4_ != 0 ||
            record.field_8_ != 0 || record.count_ != 0 ||
            record.field_C_ != 0 || record.field_14_ != 0) {
            return fail("tamper reset missed a record dword");
        }
    }
    return 0;
}

int main() {
    if (verify_offer_pairs() != 0) return 1;
    if (verify_tamper_registry() != 0) return 1;
    puts("PASS: current source bodies preserve exact reset footprints");
    return 0;
}
