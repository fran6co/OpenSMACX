#!/usr/bin/env -S uv run python
"""Scratch: name the homed bodies' parameters and drop their self-puns."""
import pathlib

p = pathlib.Path("src/netdaemon.cpp")
t = p.read_text()

old_hang = '''void NetDaemon::hang_up() {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x1b3c) != 0) {
        return;
    }
    if (NetDaemonIsMultiplayerNet == 0) {
        return;
    }
    *reinterpret_cast<int *>(self + 0x1b3c) = 1;
    if (NetTurnFlags & 0x10) {
        if (*ConsoleControlTurnActive == 0) {
            if (NetTurnFaction != NetDaemonLocalFaction) {
                goto L0cc;
            }
        } else if (NetDaemonNet.lock_state_[0].dword_[1] == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (NetDaemonNet.lock_state_[0].dword_[1] != 0) {
        reinterpret_cast<PlayerLock *>(self + NetDaemonLocalFaction * 28 + 0x14a0)->clear();
        if (*reinterpret_cast<int *>(self + 0x1580) == NetDaemonLocalFaction) {
            *reinterpret_cast<int *>(self + 0x1584) = 0;
            *reinterpret_cast<int *>(self + 0x1580) = 0;
        }
        *reinterpret_cast<unsigned int *>(self + 0x1b7c) &=
            ~*reinterpret_cast<unsigned int *>(self + 0x1b80 + NetDaemonLocalFaction * 4);
        message_data(0x4b00, 0, 0, 0, 0, 0);
    }
L145:
    NetMsgPopup->close();
}'''

new_hang = '''// 0x1B3C is lock_state_[0].dword_[4], 0x1580/0x1584 are the Lock's tail
// dwords, 0x1B7C is lock_state_[1].dword_[5] and the four dwords at 0x1B80
// are lock_state_[1].dword_[6..9] - one per faction.
void NetDaemon::hang_up() {
    if (lock_state_[0].dword_[4] != 0) {
        return;
    }
    if (NetDaemonIsMultiplayerNet == 0) {
        return;
    }
    lock_state_[0].dword_[4] = 1;
    if (NetTurnFlags & 0x10) {
        if (*ConsoleControlTurnActive == 0) {
            if (NetTurnFaction != NetDaemonLocalFaction) {
                goto L0cc;
            }
        } else if (NetDaemonNet.lock_state_[0].dword_[1] == 0) {
            goto L145;
        }
        message_data(0x8301, 0, 0, 0, 0, 0);
    }
L0cc:
    if (NetDaemonNet.lock_state_[0].dword_[1] != 0) {
        locks_.records_[NetDaemonLocalFaction].clear();
        if (locks_.field_E0_ == NetDaemonLocalFaction) {
            locks_.field_E4_ = 0;
            locks_.field_E0_ = 0;
        }
        lock_state_[1].dword_[5] &=
            ~lock_state_[1].dword_[6 + NetDaemonLocalFaction];
        message_data(0x4b00, 0, 0, 0, 0, 0);
    }
L145:
    NetMsgPopup->close();
}'''

old_eoe = '''int NetDaemon::end_of_event(int a1, int a2) {
    char *self = reinterpret_cast<char *>(this);
    if (*reinterpret_cast<int *>(self + 0x1b30) == 0) {
        *reinterpret_cast<int *>(self + 0x1b2c) = a2;
    }
    if (a1 == NetDaemonLocalFaction) {
        if (*reinterpret_cast<int *>(self + 0x1bc0) != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            *reinterpret_cast<int *>(self + 0x1bc0) = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        *reinterpret_cast<int *>(self + 0x1bb0) = 0;
    }
    return 0;
}'''

new_eoe = '''// 0x1B2C/0x1B30 are lock_state_[0].dword_[0]/[1]; 0x1BB0 and 0x1BC0 are
// lock_state_[2].dword_[3] and [7].
int NetDaemon::end_of_event(int faction_id, int turn_value) {
    if (lock_state_[0].dword_[1] == 0) {
        lock_state_[0].dword_[0] = turn_value;
    }
    if (faction_id == NetDaemonLocalFaction) {
        if (lock_state_[2].dword_[7] != 0) {
            log_say("Remaining Locked2", 0, 0, 0);
            lock_state_[2].dword_[7] = 0;
            message_data(0x2214, 0, 0, 0, 0, 0);
            return 0;
        }
        lock_state_[2].dword_[3] = 0;
    }
    return 0;
}'''

assert old_hang in t
t = t.replace(old_hang, new_hang, 1)
assert old_eoe in t
t = t.replace(old_eoe, new_eoe, 1)

p.write_text(t)
print("ok")
