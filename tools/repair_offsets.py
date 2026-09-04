#!/usr/bin/env -S uv run python
"""Scratch: repair the member conversions that left a stale deref.

`*reinterpret_cast<int *>(self + 0x1B30) = 0;` became
`*reinterpret_cast<int *>(lock_state_[0].dword_[1]) = 0;`, which writes
through the field's VALUE. Every one of those has to lose the deref.
"""
import pathlib
import re

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()

MEM = r"(?:lock_state_\[\d+\]\.dword_\[\d+\]|locks_\.field_E[048]_|locks_\.records_\[\d\]|lock_table_\[\d+\])"

# 1. the field is already the lvalue: drop the pointer cast entirely
n = 0
pat = re.compile(r"\*reinterpret_cast<[\w ]+\*>\(\s*(" + MEM + r")\s*\)")
text, k = pat.subn(lambda m: m.group(1), text)
n += k

# 2. an address-taken form
pat = re.compile(r"reinterpret_cast<[\w ]+\*>\(\s*&?\s*(" + MEM + r")\s*\)")
def addr(m):
    return "&" + m.group(1)
text, k2 = pat.subn(addr, text)

P.write_text(text)
print(f"{k} deref(s) dropped, {k2} address form(s) fixed")
