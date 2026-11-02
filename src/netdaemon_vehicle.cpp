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
#include "stdafx.h"
#include <mmsystem.h>
#include "netdaemon.h"
#include "netmsg.h"
#include "console.h"
#include "game.h"
#include "veh.h"
#include "base.h"
#include "map.h"
#include "log.h"
#include "win.h"

// The cumulative network wait counter remains owned by netdaemon.cpp.
extern int NetDaemonWaitTicks;

// Original0x00532140 performs the network half of a vehicle drop. The
// lock-state words are the existing typed entries documented above; the
// non-network action is a separate real owner in veh.cpp.
// ORIGINAL: 0x00532140 ?veh_drop@NetDaemon@@QAEHHHH@Z 0x00532140-0x00532563 FILE
// RECOVERY_BINDING: v1 {"evidence": ["git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00532140.cpp retained signature/extents and complete seed; no historical claim copied", "Coordinator full pinned original instruction decode 00532140..00532563, typed source owner fields and corrected original call/branch evidence", "recovery/tests/test_airdrop_network.py actual-body directional treaty and network reentrancy controls; not linked/runtime/game acceptance"], "reason": "Recover the true network receiver, lock/ack state, global action-mode reentrancy and local execution result; preserve early return versus release paths and leave ExitTurnLoop read-only."}
// TRIED: flat byte-offset fields on `this` (class deliberately opaque); goto-based control flow matching the two shared cleanup labels (LAB_00532357 reject path, LAB_0053254e release path); fixed two Ghidra-vs-raw-asm discrepancies (speed()/array index use a1 not a2 after the reassignment at 0x53236F; message_data arg order is a1,a2,a3). Diverges at #0 (thiscall prologue).
int NetDaemon::veh_drop(int veh_id, int x, int y) {
    if (IsMultiplayerNet == 0) {
        return action_airdrop(veh_id, x, y, 3);
    }

    if (channels_[1].leading_words_[2] != 0) {
        if (channels_[1].trailing_words_[3] != 0) return 0;
        ++channels_[1].trailing_words_[5];
        if (static_cast<int>(channels_[1].trailing_words_[5]) < 2) return 0;
        NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
        return 0;
    }

    channels_[1].trailing_words_[5] = 0;
    if (lock_veh(&veh_id, 4, x, y, 4) != 0) return 0;

    const int base_id = base_at(x, y);
    bool rejected = false;
    if (base_id >= 0) {
        const uint8_t vehicle_faction = Vehs[veh_id].faction_id;
        const uint8_t base_faction = Bases[base_id].faction_id_current;
        rejected = vehicle_faction == base_faction ||
            (PlayersData[vehicle_faction].diplo_treaties[base_faction] & 1U) != 0;
        if (!rejected) {
            const Map &tile = *map_loc(x, y);
            if ((tile.bit & 2U) != 0) {
                const int owner = owner_at(x, y);
                rejected = owner < 8 && owner >= 0;
            }
        }
        if (!rejected) rejected = add_global() == 0;

        if (!rejected) {
            if (channels_[1].trailing_words_[3] != 0) {
                Vehs[veh_id].order = 0;
                Vehs[veh_id].state &= 0xFCFFBDFFU;
                synch_veh(veh_id);
                if (IsMultiplayerNet != 0) {
                    uint32_t *acknowledged = &channels_[1].trailing_words_[0];
                    *acknowledged = 1;
                    log_say("Awaiting acknowledgement of SYNCH",
                            ConsoleGlobal.faction_id_, 0, 0);
                    const int waited = wait("AWAITEXEC", acknowledged, 1,
                                            2000);
                    NetDaemonWaitTicks += waited;
                    if (*acknowledged == 0) {
                        log_say("Received SYNCH acknowledgement",
                                ConsoleGlobal.faction_id_, 0, 0);
                    }
                    *acknowledged = 0;
                }
            }
            goto release_vehicle;
        }
    }

    if (action_airdrop(veh_id, x, y, 1) != 0) {
        const int current_speed = speed(veh_id, 0);
        const int previous_speed = Vehs[veh_id].moves_expended;
        const int difference = current_speed - previous_speed;
        if (difference >= 0 &&
            (difference > 999 || current_speed != previous_speed)) {
            message_data(0x2403, 0, veh_id, x, y, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client waiting for acknowledgement",
                        ConsoleGlobal.faction_id_, 0, 0);
                ++NetBusyCount;
                const int saved_action_mode = NetDaemonNet.channels_[0].leading_words_[3];
                const DWORD start_time = timeGetTime();
                bool showed_wait_popup = false;
                if (channels_[1].leading_words_[3] == 1) {
                    do {
                        if (ExitTurnLoop != 0) break;
                        if (!showed_wait_popup &&
                            timeGetTime() - start_time > 2000) {
                            showed_wait_popup = true;
                            NetMsgPrimary.pop("AWAITEXEC", 0, 2, nullptr);
                        }
                        wait_task();
                        receive();
                        locks_.check_global();
                        check_spock();
                    } while (channels_[1].leading_words_[3] == 1);
                    if (showed_wait_popup) NetMsgPrimary.close();
                }
                --NetBusyCount;
                NetDaemonNet.channels_[0].leading_words_[3] = saved_action_mode;
                log_say("Client received acknowledgement",
                        ConsoleGlobal.faction_id_, 0, 0);
                if (IsMultiplayerNet != 0) {
                    log_say("Client releasing lock",
                            ConsoleGlobal.faction_id_, 0, 0);
                    message_data(0x2212, 0, 0, 0, 0, 0);
                    channels_[1].leading_words_[3] = 0;
                    channels_[1].leading_words_[2] = 0;
                    channels_[1].trailing_words_[5] = 0;
                    channels_[1].trailing_words_[4] = 0;
                }
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[3] = 0;
                channels_[1].leading_words_[3] = 0;
                return 1;
            }
            return 1;
        }
    }

release_vehicle:
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    return 0;
}

// ORIGINAL: 0x00532570 ?action@NetDaemon@@QAEHHH@Z 0x00532570-0x005328BA
// RECOVERY_BINDING: v1 {"evidence": ["Pinned532570..5328BA; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00532570.cpp is a conflicting historical hint, not the product ABI.", "Entry signed order classification is distinct from the post-automated_lock vehicle/order reload. Absolute93E904 timing storage belongs to NetDaemonNet.lock_state_[1].dword_[3], while wait flags and Lock remain receiver-relative.", "Pinned NUL log/popup literals68C2DC,68C390,68C3B4,68C3C0,68C4AC,68C4C8,68C4F4,68C510,68C530,68C544,68C554; ExitTurnLoop is game owner9B2068."], "reason": "Recover the full multiplayer vehicle-order action through the real NetDaemon, vehicle and transport owners; retain its original seed identity and complete span without a matching or runtime claim."}
// TRIED: g_00669368 is a cached stdcall function pointer (timeGetTime), called twice through the same loaded pointer - matches the two `// indirect` sites.
// TRIED: nothing else tried; landed at MISMATCH #7 (original compares several fields against a zero register cached once via `xor ebx,ebx` and reused, rebuilt re-derives `== 0` per comparison; similarity 0.886).
int NetDaemon::action(int veh_id, int mode) {
    if (IsMultiplayerNet == 0) return ::action(veh_id);

    const int8_t action_type = Vehs[veh_id].order;
    int category = 0;
    enum Branch { await_exec, go_to, road_to };
    Branch branch = await_exec;

    if (action_type < 4 || action_type > 0x17) {
        if (action_type == ORDER_MOVE_TO || action_type == ORDER_AI_MOVE_TO) {
            category = 4;
            branch = go_to;
        } else if (action_type == ORDER_ROAD_TO || action_type == ORDER_MAGTUBE_TO) {
            category = 4;
            branch = road_to;
        } else {
            if (action_type != 0)
                log_say("Falling out of action loop", veh_id, action_type, 0);
            if (mode != 0) {
                if (IsMultiplayerNet != 0) {
                    log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                    message_data(0x2212, 0, 0, 0, 0, 0);
                    channels_[1].leading_words_[3] = 0;
                    channels_[1].leading_words_[2] = 0;
                    channels_[1].trailing_words_[5] = 0;
                    channels_[1].trailing_words_[4] = 0;
                }
                channels_[0].payload_.vehicle_selection_follow = 0;
                channels_[1].trailing_words_[3] = 0;
            }
            return 0;
        }
    } else {
        category = (action_type == 0x14 || action_type == 0x15) ? 0xC : 0;
    }

    channels_[1].trailing_words_[3] = 1;
    if (mode == 0 && automated_lock(&veh_id, category) != 0) return 0;

    if (branch == await_exec) {
        log_say("Net.Action Sending ACTION message to server", veh_id,
                Vehs[veh_id].order, 0);
        message_veh(0x2404, veh_id, 0, 0);
        log_say("Net.Action Awaiting EXEC", 0, 0, 0);
        if (IsMultiplayerNet != 0) {
            log_say("Client waiting for acknowledgement",
                    ConsoleGlobal.faction_id_, 0, 0);
            const uint32_t saved_action_mode =
                NetDaemonNet.channels_[0].leading_words_[3];
            DWORD (WINAPI *const get_time)() = timeGetTime;
            ++NetBusyCount;
            const DWORD start_time = get_time();
            bool showed_wait_popup = false;
            if (channels_[1].leading_words_[3] == 1) {
                do {
                    if (ExitTurnLoop != 0) break;
                    if (!showed_wait_popup && get_time() - start_time > 2000) {
                        showed_wait_popup = true;
                        NetMsgPrimary.pop("AWAITEXEC", 0, 2, nullptr);
                    }
                    wait_task();
                    unsigned long first, second;
                    void *message = NetDaemonNet.Net::get(&first, &second);
                    if (message != 0)
                        process_message(static_cast<char *>(message),
                                        second, static_cast<int>(first));
                    locks_.check_global();
                    check_spock();
                } while (channels_[1].leading_words_[3] == 1);
                if (showed_wait_popup) NetMsgPrimary.close();
            }
            --NetBusyCount;
            NetDaemonNet.channels_[0].leading_words_[3] = saved_action_mode;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                channels_[1].leading_words_[3] = 0;
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[5] = 0;
                channels_[1].trailing_words_[4] = 0;
            }
            channels_[0].payload_.vehicle_selection_follow = 0;
            channels_[1].trailing_words_[3] = 0;
            channels_[1].leading_words_[3] = 0;
        }
        log_say("Net.Action Finished awaiting", 0, 0, 0);
    } else if (branch == go_to) {
        log_say("Net.Action Go to", veh_id, 0, 0);
        action_go_to(veh_id);
    } else {
        log_say("Net.Action Road", 0, 0, 0);
        action_road_to(veh_id);
    }
    log_say("Falling out of Net.Action", 0, 0, 0);
    return 1;
}

// ORIGINAL: 0x00531780 ?order_veh@NetDaemon@@QAEHHHH@Z 0x00531780-0x00532135
// RECOVERY_BINDING: v1 {"evidence": ["Pinned531780..532135; retained git:65b567af55eb22d163bbd28afcc335c340847278:src/unrecovered/00531780.cpp preserves the original identity and contradictory seed history.", "Native5318D6..5318E8 computes(otherFaction+2099*ownFaction)*4+96C9F8: PlayersData[ownFaction].diplo_treaties[otherFaction], stride20CC, not the seed20C8 transcription.", "Veh34/Base134/Map2C fields and existing coordinate arrays own the accessed state; signed speed and lock cleanup/callback ordering retain native obligations. Literal68C480 is GLOBALFAILED and68C490 is Sending MOVE VEH message; native synchronization and the full message dispatcher remain unfinished."], "reason": "Recover the complete network move request, feasibility, acknowledgement and release paths on the genuine daemon and game records. Preserve unresolved synch/process_message dependencies rather than forwarding this body to the original executable."}
// TRIED: Ghidra pseudocode used as primary source with call-signature/arg-order fixes confirmed against raw disasm (base_at, veh_at, stack_check, stack_fix, order_veh [both the free function at 0x5947C0 and the recursive-looking unqualified call, which needed `::order_veh` to avoid binding to the member of the same name], message_veh, Net::get/process_message argument mapping). Factored the 3x near-identical "release lock" block into a file-scope #define/#undef macro rather than a helper function. One table access (DAT_0096c9f8 with a stride of 0x20c8 rather than the 0x20cc seen everywhere else in this codebase) is transcribed as Ghidra states it but NOT independently re-derived from raw disasm - flagging this as the weakest point if it mismatches. Compiles; first divergence at instr #34. Not chased to byte-exact given size (2485 bytes).
int NetDaemon::order_veh(int veh_id, int direction, int mode) {
    if (IsMultiplayerNet == 0) return ::order_veh(veh_id, direction, 3);

    if (mode == 0) {
        if (channels_[1].leading_words_[2] != 0) {
            if (channels_[1].trailing_words_[3] != 0) return 0;
            ++channels_[1].trailing_words_[5];
            if (static_cast<int>(channels_[1].trailing_words_[5]) < 2) return 0;
            NetMsgPrimary.pop("MOVEPENDING", 0, 2, nullptr);
            return 0;
        }
        channels_[1].trailing_words_[5] = 0;
        if (lock_veh(&veh_id, 4, -1, -1, 0) != 0) return 0;
    }

    int x = Vehs[veh_id].x + VehMoveDeltaX[direction];
    if ((MapIsFlat & 1) == 0) {
        if (x < 0) x += MapLongitudeBounds;
        else if (x >= MapLongitudeBounds) x -= MapLongitudeBounds;
    }
    const int faction = Vehs[veh_id].faction_id;
    const int y = Vehs[veh_id].y + VehMoveDeltaY[direction];
    const int base_id = base_at(x, y);
    bool good_base = false;
    Vehs[veh_id].probe_action = 0;

    if (base_id >= 0) {
        const int base_faction = Bases[base_id].faction_id_current;
        if (base_faction != faction &&
            (PlayersData[faction].diplo_treaties[base_faction] & 1U) == 0) {
            const Map &tile = MapTiles[MapLongitude * y + (x >> 1)];
            if ((tile.bit & 2U) == 0 || (tile.val2 & 0x0F) > 7 ||
                Bases[base_id].population_size < 2) {
                good_base = true;
            }
        }
    }

    bool do_order;
    if (planet_buster(veh_id) == 0) {
        do_order = good_base;
    } else {
        const int fixed_vehicle = stack_fix(veh_at(x, y));
        if (fixed_vehicle < 0 || Vehs[fixed_vehicle].faction_id == faction ||
            (PlayersData[faction]
                 .diplo_treaties[Vehs[fixed_vehicle].faction_id] & 1U) != 0) {
            do_order = good_base;
        } else {
            do_order = true;
        }
    }

    if (do_order && add_global() != 0) {
        if (channels_[1].trailing_words_[3] != 0) {
            Vehs[veh_id].order = 0;
            Vehs[veh_id].state &= 0xFCFFBDFFU;
            synch_veh(veh_id);
            if (IsMultiplayerNet != 0) {
                uint32_t *flag = &channels_[1].trailing_words_[0];
                *flag = 1;
                log_say("Awaiting acknowledgement of SYNCH", ConsoleGlobal.faction_id_, 0, 0);
                const int waited = wait("AWAITEXEC", flag, 1, 2000);
                NetDaemonWaitTicks += waited;
                if (*flag == 0)
                    log_say("Received SYNCH acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
                *flag = 0;
            }
        }
        if (IsMultiplayerNet != 0) {
            log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
            message_data(0x2212, 0, 0, 0, 0, 0);
            channels_[1].leading_words_[3] = 0;
            channels_[1].leading_words_[2] = 0;
            channels_[1].trailing_words_[5] = 0;
            channels_[1].trailing_words_[4] = 0;
        }
        channels_[0].payload_.vehicle_selection_follow = 0;
        channels_[1].trailing_words_[3] = 0;
        return 0;
    }

    if (::order_veh(veh_id, direction, 1) == 0) goto release_lock_only;

    {
        const int difference = speed(veh_id, 0) - Vehs[veh_id].moves_expended;
        if (difference < 0 ||
            (difference < 1000 && difference == 0))
            goto release_lock_only;
    }

    if (VehPrototypes[Vehs[veh_id].proto_id].plan == PLAN_INFO_WARFARE &&
        base_id >= 0 && faction != Bases[base_id].faction_id_current) {
        const int stack_vehicle = veh_at(x, y);
        if (stack_check(stack_vehicle, 2, PLAN_INFO_WARFARE, -1, -1) == 0 &&
            !good_base) {
            if (add_global() == 0) goto send_move;
            goto global_failed;
        }
    }

    if (base_id < 0 || Bases[base_id].population_size > 1 ||
        (Vehs[veh_id].state & 0x80000U) == 0 || good_base || add_global() == 0) {
send_move:
        log_say("Sending MOVE VEH message", veh_id, direction, 0);
        message_veh(0x2401, veh_id, direction, 0);
        if (mode == 0 && IsMultiplayerNet != 0) {
            log_say("Client waiting for acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            uint32_t *flag = &channels_[1].leading_words_[3];
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            log_say("Client received acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            if (IsMultiplayerNet != 0) {
                log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
                message_data(0x2212, 0, 0, 0, 0, 0);
                *flag = 0;
                channels_[1].leading_words_[2] = 0;
                channels_[1].trailing_words_[5] = 0;
                channels_[1].trailing_words_[4] = 0;
            }
            channels_[0].payload_.vehicle_selection_follow = 0;
            channels_[1].trailing_words_[3] = 0;
            *flag = 0;
        }

        bool have_other_vehicle = false;
        int other_faction_found = 0;
        for (int byte_offset = 0; byte_offset <= 0x53; byte_offset += 4) {
            int adjacent_x = RadiusOffsetX[byte_offset / 4] + x;
            if ((MapIsFlat & 1) == 0) {
                if (adjacent_x < 0) adjacent_x += MapLongitudeBounds;
                else if (adjacent_x >= MapLongitudeBounds) adjacent_x -= MapLongitudeBounds;
            }
            const int adjacent_y = RadiusOffsetY[byte_offset / 4] + y;
            if (adjacent_y >= 0 && adjacent_y < MapLatitudeBounds &&
                adjacent_x >= 0 && adjacent_x < MapLongitudeBounds) {
                const Map &tile = MapTiles[MapLongitude * adjacent_y + (adjacent_x >> 1)];
                if ((tile.bit & 3U) != 0) {
                    const int other_faction = tile.val2 & 0x0F;
                    if (other_faction < 8 && other_faction != ConsoleGlobal.faction_id_ &&
                        (PlayersData[other_faction]
                             .diplo_treaties[ConsoleGlobal.faction_id_] & 0x10U) != 0 &&
                        (GameDataStateStorage.fields_.lock_masks_.enable_mask_ & (1U << other_faction)) != 0) {
                        int other_vehicle = veh_at(adjacent_x, adjacent_y);
                        while (other_vehicle >= 0) {
                            if (Vehs[other_vehicle].faction_id == other_faction) {
                                const int remaining = speed(other_vehicle, 0) -
                                    Vehs[other_vehicle].moves_expended;
                                if (remaining >= 0 &&
                                    (remaining > 999 || remaining != 0)) {
                                    have_other_vehicle = true;
                                    other_faction_found = other_faction;
                                }
                            }
                            other_vehicle = Vehs[other_vehicle].prev_veh_id_stack;
                        }
                    }
                }
            }
        }
        (void)other_faction_found;

        if (have_other_vehicle) {
            DWORD (WINAPI *const get_time)() = timeGetTime;
            const DWORD start = get_time();
            do {
                do_all_non_input();
                unsigned long first, second;
                void *message = NetDaemonNet.Net::get(&first, &second);
                if (message != 0)
                    process_message(static_cast<char *>(message), second,
                                    static_cast<int>(first));
                locks_.check_global();
                if (ExitTurnLoop != 0) break;
            } while (get_time() - start < 500);
        }
        return 1;
    }

    if (channels_[1].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            uint32_t *flag = &channels_[1].trailing_words_[0];
            *flag = 1;
            log_say("Awaiting acknowledgement of SYNCH", ConsoleGlobal.faction_id_, 0, 0);
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            if (*flag == 0)
                log_say("Received SYNCH acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            *flag = 0;
        }
    }
release_lock_only:
    // Native 0x005320CF is the short failure tail used by a rejected order
    // or exhausted movement. It does not clear/synchronize the vehicle.
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    return 0;

global_failed:
    // Kept separate because the seed's add_global failure path pops before
    // returning; it does not perform the generic release helper above.
    if (channels_[1].trailing_words_[3] != 0) {
        Vehs[veh_id].order = 0;
        Vehs[veh_id].state &= 0xFCFFBDFFU;
        synch_veh(veh_id);
        if (IsMultiplayerNet != 0) {
            uint32_t *flag = &channels_[1].trailing_words_[0];
            *flag = 1;
            log_say("Awaiting acknowledgement of SYNCH", ConsoleGlobal.faction_id_, 0, 0);
            const int waited = wait("AWAITEXEC", flag, 1, 2000);
            NetDaemonWaitTicks += waited;
            if (*flag == 0)
                log_say("Received SYNCH acknowledgement", ConsoleGlobal.faction_id_, 0, 0);
            *flag = 0;
        }
    }
    if (IsMultiplayerNet != 0) {
        log_say("Client releasing lock", ConsoleGlobal.faction_id_, 0, 0);
        message_data(0x2212, 0, 0, 0, 0, 0);
        channels_[1].leading_words_[3] = 0;
        channels_[1].leading_words_[2] = 0;
        channels_[1].trailing_words_[5] = 0;
        channels_[1].trailing_words_[4] = 0;
    }
    channels_[0].payload_.vehicle_selection_follow = 0;
    channels_[1].trailing_words_[3] = 0;
    NetMsgPrimary.pop("GLOBALFAILED", 5000, 0, nullptr);
    return 0;
}
