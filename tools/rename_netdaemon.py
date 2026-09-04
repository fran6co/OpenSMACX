#!/usr/bin/env -S uv run python
"""Scratch: evidence-name the fixed-address bindings in src/netdaemon.cpp."""
import re
import pathlib

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()

# 1. includes the homed bodies need
old_inc = '''#include "stringstruct.h"  // StringAllocationHeap - the 0x009B3374 hand-off
'''
new_inc = '''#include "stringstruct.h"  // StringAllocationHeap - the 0x009B3374 hand-off
#include "netmsg.h"        // NetMsgPopup, which the lock/wait bodies pop through
#include "basepop.h"       // NetMsg's base, which process_message reaches
#include "veh.h"           // Vehs / VehCurrentCount, which game_checksum walks
#include "map.h"           // the map bounds, PlayersData and Bases it sums
#include "technology.h"    // GameTechAchieved
'''
assert old_inc in text
text = text.replace(old_inc, new_inc, 1)

# 2. drop the NetMsg shim - the real class is in netmsg.h
shim = '''class NetMsg { public:
    void close();
};

'''
assert shim in text
text = text.replace(shim, "", 1)

# 3. delete every carried anonymous binding, with its "carried from" header
text = re.sub(r"// Fixed-slot bindings carried from \w+\.cpp\n"
              r"(?:static int \*const g_[0-9a-f]+ = \(int \*\)0x[0-9A-Fa-f]+;\n)*",
              "", text)
text = re.sub(r"^static int \*const g_[0-9a-f]+ = \(int \*\)0x[0-9A-Fa-f]+;\n",
              "", text, flags=re.M)

# 4. the Net* receiver - the tree's idiom, as receive() already spells it
text = text.replace("((Net *)NetDaemonNet)->get(", "NetDaemonNet.Net::get(")
text = text.replace(
    "reinterpret_cast<Net *>(NetDaemonNet)->get(", "NetDaemonNet.Net::get(")
text = text.replace("reinterpret_cast<NetMsg *>(NetMsgPopup)->",
                    "NetMsgPopup->")

# 5. plain identifier renames
PLAIN = {
    "g_00939284": "NetDaemonLocalFaction",
    "g_0093f660": "NetDaemonIsMultiplayerNet",
    "g_0093a938": "*ConsoleControlTurnActive",
    "g_0093a94c": "BaseAtDebugPopupFlag",
    "g_009a681c": "NetTurnFlags",
    "g_009a6820": "NetTurnFaction",
    "g_009a64d4": "TurnCurrentNum",
    "g_00949870": "MapLongitudeBounds",
    "g_00949874": "MapLatitudeBounds",
    "g_00949884": "MapArea",
    "g_0094a2a0": "MapSizePlanet",
    "g_0094a30c": "LockMapTable",
    "g_0068faf0": "*MapRowStride",
    "g_00952828": "(char *)Vehs",
    "g_0095282a": "((char *)Vehs + 2)",
    "g_009a649c": "(char *)&GameRules",
    "g_009a64c8": "(char *)&VehCurrentCount",
    "g_009a64cc": "BaseCurrentCount",
    "g_009a64e8": "(char *)&LockEnableMask",
    "g_009a6514": "(char *)&SecretProject",
    "g_009a6670": "GameTechAchieved",
    "g_009469f8": "ReplayEventCount",
    "g_0093e8c0": "NetDaemonNet.lock_state_[0].dword_[1]",
    "g_0093e8bc": "NetDaemonNet.lock_state_[0].dword_[0]",
    "g_0093e8ec": "NetDaemonNet.lock_state_[0].dword_[12]",
    "g_0093e904": "NetDaemonNet.lock_state_[1].dword_[3]",
    "g_0093e93c": "NetDaemonNet.lock_state_[2].dword_[2]",
    "g_0093f664": "NetBusyCount",
    "g_0096ee78": "((char *)PlayersData + 0x2498)",
    "g_0097d07c": "((char *)Bases + 0x3C)",
    "g_0097d40c": "((char *)Bases + 0x3CC)",
    "g_0068c570": '"Remaining Locked2"',
    "g_0068c340": '"ALREADYDONE"',
}
for addr, name in PLAIN.items():
    text = re.sub(rf"\b{addr}\b", name, text)

# 6. tidy the casts the renames left behind
text = text.replace("(char *)(char *)", "(char *)")
text = text.replace("(char *)(char *)&", "(char *)&")
text = text.replace("checksum((char *)MapArea", "checksum((char *)&MapArea")
text = text.replace("(char *)(*MapLongitudeBounds)", "(char *)&MapLongitudeBounds")
text = text.replace("(char *)(*MapLatitudeBounds)", "(char *)&MapLatitudeBounds")
text = text.replace("(char *)(*LockMapTable)", "(char *)LockMapTable")
text = text.replace("(char *)(*MapRowStride)", "(char *)MapRowStride")
text = text.replace("*NetTurnFlags", "NetTurnFlags")
text = text.replace("(char *)MapSizePlanet", "(char *)&MapSizePlanet")
text = text.replace("(char *)(char *)", "(char *)")

P.write_text(text)
print("rewritten")
for line in text.splitlines():
    if "g_00" in line:
        print("  LEFT:", line.strip())
