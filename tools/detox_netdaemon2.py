#!/usr/bin/env -S uv run python
"""Scratch: name the six bindings the 12 newly homed bodies still carry."""
import pathlib
import re

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()

# --- the timeGetTime IAT slot, through net_class.h's accessor -------------
patterns = [
    (r"\(\*reinterpret_cast<\w+ \*>\(g_00669368\)\)\(\)", "NetTimeGetTimeSlot()()"),
    (r"\(\*\(PTimeGetTime \*\)g_00669368\)\(\)", "NetTimeGetTimeSlot()()"),
    (r"\*reinterpret_cast<\w+ \*>\(g_00669368\)", "NetTimeGetTimeSlot()"),
    (r"reinterpret_cast<\w+>\(\*g_00669368\)", "NetTimeGetTimeSlot()"),
]
for pat, rep in patterns:
    text = re.sub(pat, rep, text)

PLAIN = {
    # 0x1B74 - the dword the lock bodies save and restore around their wait.
    "g_0093e904": "NetDaemonNet.lock_state_[1].dword_[3]",
    # 0x1BAC - the lock-held flag unlock() also writes.
    "g_0093e93c": "NetDaemonNet.lock_state_[2].dword_[2]",
    # 0x93F65C - four bytes below NetDaemonIsMultiplayerNet; on_player_deleted
    # sets it, and a lock whose wait saw it give up with "NOLOCK".
    "g_0093f65c": "NetPlayerDropped",
    # commlink_attempt (0x00558C60) sets these together: the flag and the
    # faction that asked for the commlink. The daemon's dispatch tests both.
    "g_007492cc": "CommlinkPending",
    "g_0073b790": "CommlinkFaction",
}
for addr, name in PLAIN.items():
    text = re.sub(rf"\b{addr}\b", name, text)

text = re.sub(r"^static int \*const g_[0-9a-f]+ = \(int \*\)0x[0-9A-Fa-f]+;\n",
              "", text, flags=re.M)

# definitions for the two names this file now owns
text = text.replace(
    "int ReplayEventCount;  // 0x009469F8",
    """// Four bytes below NetDaemonIsMultiplayerNet. on_player_deleted
// (0x0052E370) sets it when a client leaves; a lock whose wait saw it comes
// back with the "NOLOCK" popup instead of a lock.
int NetPlayerDropped;  // 0x0093F65C

// Set as a pair by commlink_attempt (0x00558C60): the flag and the faction
// that asked. process_message tests both when a commlink reply arrives.
int CommlinkPending;   // 0x007492CC
int CommlinkFaction;   // 0x0073B790

int ReplayEventCount;  // 0x009469F8""", 1)

P.write_text(text)
left = sorted({m for m in re.findall(r"\bg_[0-9a-f]{6,8}\b", text)})
print(f"{len(left)} anonymous identifier(s) left: {left}")
