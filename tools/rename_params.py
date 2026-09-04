#!/usr/bin/env -S uv run python
"""Scratch: name the daemon bodies' parameters, function by function."""
import pathlib
import re

P = pathlib.Path("src/netdaemon.cpp")
text = P.read_text()
lines = text.splitlines(keepends=True)


def find(header_re):
    for i, ln in enumerate(lines):
        if re.search(header_re, ln):
            end = i
            for j in range(i + 1, len(lines)):
                if lines[j].startswith("}"):
                    end = j
                    break
            return i, end
    raise SystemExit(f"header not found: {header_re}")


def rename(start, end, mapping, decl_old, decl_new):
    for i in range(start, end + 1):
        for old, new in mapping.items():
            lines[i] = re.sub(rf"\b{old}\b", new, lines[i])
    lines[start] = lines[start].replace(decl_old, decl_new)


# NetDaemon::init - the option byte setup() is handed
s, e = find(r"int NetDaemon::init\(int a1\)")
rename(s, e, {"a1": "option"}, "(int a1)", "(int option)")

# NetDaemon::wait(message, state, value, timeout)
s, e = find(r"int NetDaemon::wait\(char \* a1, int \* a2, int a3, int a4\)")
rename(s, e, {"a1": "message", "a2": "state", "a3": "value", "a4": "timeout"},
       "(char * a1, int * a2, int a3, int a4)",
       "(char *message, int *state, int value, int timeout)")

# NetDaemon::open_channel(payload_10, payload_14)
s, e = find(r"int NetDaemon::open_channel\(int a1, int a2\)")
rename(s, e, {"a1": "payload_10", "a2": "payload_14"},
       "(int a1, int a2)", "(int payload_10, int payload_14)")

# NetDaemon::lock(target, x, y, payload_18, payload_1C, payload_20)
s, e = find(r"int NetDaemon::lock\(int a1, int a2, int a3, int a4, int a5, int a6\)")
rename(s, e, {"a1": "target", "a2": "x", "a3": "y", "a4": "payload_18",
              "a5": "payload_1C", "a6": "scratch"},
       "(int a1, int a2, int a3, int a4, int a5, int a6)",
       "(int target, int x, int y, int payload_18, int payload_1C, "
       "int payload_20)")

# NetDaemon::add_lock(target, x, y)
s, e = find(r"int NetDaemon::add_lock\(int a1, int a2, int a3\)")
rename(s, e, {"a1": "target", "a2": "x", "a3": "y"},
       "(int a1, int a2, int a3)", "(int target, int x, int y)")

s, e = find(r"void NetDaemon::unlock_base\(int a1\)")
rename(s, e, {"a1": "base_id"}, "(int a1)", "(int base_id)")

s, e = find(r"void NetDaemon::await_exec\(int a1\)")
rename(s, e, {"a1": "opcode"}, "(int a1)", "(int opcode)")

s, e = find(r"void NetDaemon::await_diplo\(int a1\)")
rename(s, e, {"a1": "opcode"}, "(int a1)", "(int opcode)")

s, e = find(r"int NetDaemon::fixup_message\(char \* a1\)")
rename(s, e, {"a1": "message"}, "(char * a1)", "(char *message)")

s, e = find(r"void NetDaemon::process_message\(char \* a1, unsigned long a2, int a3\)")
rename(s, e, {"a1": "message", "a2": "sender", "a3": "channel"},
       "(char * a1, unsigned long a2, int a3)",
       "(char *message, unsigned long sender, int channel)")

P.write_text("".join(lines))
left = sorted({m.group(0) for m in re.finditer(r"\ba\d\b", "".join(lines))})
print("remaining aN identifiers:", left)
for i, ln in enumerate(lines):
    if re.search(r"\ba\d\b", ln):
        print(i + 1, ln.rstrip()[:100])
