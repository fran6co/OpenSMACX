#!/usr/bin/env -S uv run python
"""Scratch: name the daemon's own fields in place of `self + offset`.

The lock block is modelled (locks_ at 0x14A0, lock_table_ at 0x158C,
lock_state_ at 0x1B2C), so every raw offset inside those three arrays is a
field the class already declares.
"""
import pathlib
import re

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()

LOCKS = 0x14A0
TABLE = 0x158C
STATE = 0x1B2C
STATE_END = STATE + 24 * 0x3C


def state_member(off):
    delta = off - STATE
    entry, rest = divmod(delta, 0x3C)
    if rest % 4:
        return None
    return f"lock_state_[{entry}].dword_[{rest // 4}]"


def table_member(off):
    delta = off - TABLE
    entry, rest = divmod(delta, 0x3C)
    if rest % 1 or entry > 23 or rest >= 0x3C:
        return None
    return f"lock_table_[{entry}]" + (f".{hex(rest)[2:]}_" if rest else "")


def locks_member(off):
    delta = off - LOCKS
    if delta % 0x1C == 0 and delta // 0x1C < 8:
        return f"locks_.records_[{delta // 0x1C}]"
    return {0xE0: "locks_.field_E0_", 0xE4: "locks_.field_E4_",
            0xE8: "locks_.field_E8_"}.get(delta)


def member(off):
    if STATE <= off < STATE_END:
        return state_member(off)
    if TABLE <= off < STATE:
        m = table_member(off)
        return m or f"*reinterpret_cast<uint8_t *>(self + {hex(off)})"
    if LOCKS <= off < TABLE:
        return locks_member(off)
    return None


hits = 0


def sub(m):
    global hits
    off = int(m.group("off"), 16)
    name = member(off)
    if name is None:
        return m.group(0)
    hits += 1
    return name


# the two shapes the bodies use: `self + 0xNNN` on its own, and the
# `(int *)(self + 0xNNN)` / `(char *)(self + 0xNNN)` idiom
text = re.sub(r"self \+ (?P<off>0x[0-9a-fA-F]+)", sub, text)
P.write_text(text)
print(f"{hits} offset(s) named")
