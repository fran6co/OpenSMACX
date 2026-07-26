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

/*
 * Direct-source tests for the gameplay core.
 *
 * This binary exists because `recovery-leaf-tests` cannot host these. That
 * suite defines file-scope stand-ins for map.cpp and general.cpp symbols -
 * a scriptable `mem_get`, call-recording `lock_map`/`unlock_map`, `xrange`,
 * `purge_spaces` - which its own fixtures assert against. Linking the real
 * translation units there would collide with every one of those observers.
 *
 * So the split is by link closure, not by subject: this target links the real
 * gameplay tree (map, veh, base, faction, alpha, general, technology, path,
 * probe, council, scenario, terraforming, game) and the support classes it
 * pulls in, and therefore has no stand-ins to collide with. That closure is
 * self-satisfying: the only symbols it does not resolve internally are Win32
 * imports, which is why gdi32 and winmm are linked, and the deferred oracle
 * entry point, which is stubbed below.
 *
 * Free functions in these files were previously deferred as "no home in a
 * linked file". That is what this target removes.
 */

#include "../src/stdafx.h"
#include "../src/game.h"
#include "recovery_fixtures.h"

using recovery_fixtures::expect;
using recovery_fixtures::failure_count;

/*
 * The real deferred oracle phase runs inside the hybrid process, after the
 * executable's CRT is up, and drives proprietary fixtures. Linking it here
 * would drag in every oracle suite; the scenario call site is exercised by the
 * hybrid smoke gate instead.
 */
void run_deferred_oracles() { }

namespace {

// Rebind a global pointer seam for the duration of a check and put it back,
// so one test cannot leak state into the next.
template <typename T>
class ScopedSeam {
 public:
    ScopedSeam(T **seam, T *replacement) : seam_(seam), saved_(*seam) {
        *seam_ = replacement;
    }
    ~ScopedSeam() { *seam_ = saved_; }

 private:
    T **seam_;
    T *saved_;
};

void test_not_my_turn() {
    BOOL is_net = 0;
    uint8_t flags = 0;
    int net_faction = 0;
    int local_faction = 0;

    ScopedSeam<BOOL> net_seam(&IsMultiplayerNet, &is_net);
    ScopedSeam<uint8_t> flag_seam(&NetTurnFlags, &flags);
    ScopedSeam<int> turn_seam(&NetTurnFaction, &net_faction);
    ScopedSeam<int> local_seam(&LocalFaction, &local_faction);

    // Not a net game: neither the flag nor the faction mismatch matters, and
    // the answer is "my turn" regardless.
    is_net = 0;
    flags = 0xFF;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == false);

    // A net game that is not currently passing the turn: still my turn, even
    // with the factions disagreeing. Only bit 4 arms the comparison.
    is_net = 1;
    flags = 0xEF;  // every bit but 4
    expect(not_my_turn() == false);

    // Armed, and the active faction is someone else.
    flags = 0x10;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == true);

    // Armed, and the active faction is the local one.
    net_faction = 5;
    expect(not_my_turn() == false);

    // The comparison is an inequality on the faction identity, so it fires for
    // a lower id just as it does for a higher one, and zero is an ordinary id
    // rather than a sentinel.
    net_faction = 0;
    local_faction = 1;
    expect(not_my_turn() == true);
    net_faction = 0;
    local_faction = 0;
    expect(not_my_turn() == false);
    net_faction = 7;
    local_faction = 0;
    expect(not_my_turn() == true);

    // A truthy net flag other than 1 still counts: the original tests the
    // dword for nonzero, it does not compare it against TRUE.
    is_net = 0x1234;
    net_faction = 3;
    local_faction = 5;
    expect(not_my_turn() == true);

    // Every other flag bit set alongside bit 4 leaves it armed.
    flags = 0xFF;
    expect(not_my_turn() == true);
}

}  // namespace

int main() {
    test_not_my_turn();
    if (failure_count() != 0) {
        std::fprintf(stderr, "recovery-gameplay-tests: %d failure(s)\n",
                     failure_count());
    }
    return failure_count() == 0 ? 0 : 1;
}
