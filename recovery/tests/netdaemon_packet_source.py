"""Strict inverse of the unchanged packet builder's source-home migration.

Read actual new owners, never substitute baseline bodies into the candidate.
Historical callers can retain their immutable identity/token reversal controls.
"""
from functools import lru_cache
from pathlib import Path
import re
import subprocess

from match_recovery.source import read_text
from test_infowin_setup_text import _function_end

BASE = "bffedcf01d88ec837f56df4762bdcbb6ce5eff97"
INTERFACE_BASE = "9c2b8717430cefdb6c7caca356e32d2609a9cffe"
INCLUDE = '#include "netdaemon_message_record.h"'
RECORD_START = "// The two bytes after opcode are observed unwritten"
RECORD_END = "// Borrowed wire view over the layout already owned"
SIGNATURE = "int __cdecl message_data("
NEXT = "// Native automatic wire records. Bytes 2..3 remain unwritten"
SPOCK_INSERTION = "// ORIGINAL: 0x00530360 ?net_tasks@NetDaemon@@QAEXXZ"
SPOCK_OWNERS = {
    0x0051F1B0: "void __cdecl check_spock() {",
    0x0051F0C0: "void __cdecl help_spock() {",
}
INTERFACE_OWNERS = {
    0x0051F2C0: "void __cdecl wait_2() {",
    0x00593220: "void __cdecl synch_veh(int id) {",
    0x00593250: "void __cdecl synch_base(int id) {",
    0x00593280: "void __cdecl synch_energy(int id) {",
    0x005932B0: "void __cdecl synch_researching(int id) {",
    0x005932E0: "void __cdecl synch_leader(int id) {",
    0x00593310: "void __cdecl synch_ai(int id) {",
    0x00593340: "void __cdecl synch_research(int id) {",
    0x00593390: "void __cdecl synch_alloc(int id) {",
    0x005933C0: "void __cdecl synch_soc(int id) {",
    0x005933F0: "void __cdecl synch_proto(int id) {",
    0x00593420: "void __cdecl synch_obs(int id) {",
    0x00593450: "void __cdecl synch_diplo(int a, int b) {",
    0x005934B0: "void __cdecl synch_template(int id) {",
    0x005934E0: "void __cdecl synch_radius(int id) {",
}
INTERFACE_INSERTIONS = {
    0x0051F2C0: "/*\nPurpose: Tear down a network daemon through its lock members and the\n         AlphaNet/Net destructor chain.",
    0x00593220: "/*\nPurpose: Release the vehicle lock this client is holding.",
}


@lru_cache(maxsize=None)
def _baseline(root):
    return subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"],
                                   cwd=root, text=True, timeout=30)


@lru_cache(maxsize=None)
def _interface_baseline(root):
    return subprocess.check_output(["git", "show", INTERFACE_BASE + ":src/netdaemon.cpp"],
                                   cwd=root, text=True, timeout=30)


def _without_comments(text):
    # Preserve strings, especially include filenames, when stripping comments.
    pattern = r'"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|//[^\n]*|/\*.*?\*/'
    return re.sub(pattern, lambda m: "" if m[0].startswith(("//", "/*")) else m[0], text, flags=re.S)


def _envelope(text, includes, label):
    code = _without_comments(text)
    assert [s.strip() for s in code.splitlines() if s.strip()] == includes, label
    assert "ORIGINAL:" not in text and "RECOVERY_BINDING:" not in text, label + " extra metadata"
    assert not re.search(r"(?m)^\s*//\s*(?:symbol|body|compile_context)\b", text), label + " orphan metadata fact"


def sender_record_block(root, header_text=None):
    root = Path(root)
    text = (root / "src/netdaemon_message_record.h").read_text() if header_text is None else header_text
    baseline = _baseline(root)
    expected = baseline[baseline.index(RECORD_START):baseline.index(RECORD_END)].rstrip()
    assert text.count(RECORD_START) == 1, "record rationale must have sole owner"
    start = text.index(RECORD_START)
    block = text[start:].rstrip()
    assert block == expected, "complete actual record/rationale/eight assertions unchanged"
    _envelope(text[:start], ["#pragma once", "#include <stdint.h>", "#include <stddef.h>"], "record header envelope")
    assert block.count("static_assert(") == 8
    assert not read_text(text, root / "src/netdaemon_message_record.h")
    return block + "\n\n"


def sender_block(root, sender_text=None):
    root = Path(root)
    text = (root / "src/netdaemon_message_data.cpp").read_text() if sender_text is None else sender_text
    baseline = _baseline(root)
    signature = baseline.index(SIGNATURE)
    marker = baseline.rfind("// ORIGINAL:", 0, signature)
    expected = baseline[marker:_function_end(baseline, signature)]
    native_marker = expected.splitlines()[0]
    assert text.count(native_marker) == 1, "sole genuine sender mapping"
    start = text.index(native_marker)
    body = text[start:].rstrip()
    assert body == expected, "complete actual sender body/identity/binding/history unchanged"
    _envelope(text[:start], ['#include "stdafx.h"', '#include <mmsystem.h>',
              '#include "netdaemon.h"', '#include "console.h"', INCLUDE], "sender TU envelope")
    records = read_text(text, root / "src/netdaemon_message_data.cpp")
    assert len(records) == 1 and records[0].address == 0x592EE0
    return body + "\n\n"


def _spock_blocks(root, spock_text=None):
    """Read and validate both moved annotated bodies against immutable history."""
    root = Path(root)
    path = root / "src/netdaemon_spock.cpp"
    text = path.read_text() if spock_text is None else spock_text
    baseline = _baseline(root)

    blocks = {}
    baseline_positions = {}
    candidate_positions = {}
    for address, signature in SPOCK_OWNERS.items():
        marker = f"// ORIGINAL: 0x{address:08X}"
        assert text.count(marker) == 1, f"Spock identity {address:08X} must have one actual owner"
        assert baseline.count(marker) == 1, f"immutable Spock identity {address:08X} missing/ambiguous"
        start = text.index(marker)
        end = _function_end(text, text.index(signature, start))
        block = text[start:end]
        base_start = baseline.index(marker)
        base_end = _function_end(baseline, baseline.index(signature, base_start))
        expected = baseline[base_start:base_end]
        assert block == expected, f"Spock {address:08X} annotation/body differs from immutable history"
        blocks[address] = block
        baseline_positions[address] = base_start
        candidate_positions[address] = start

    assert sorted(candidate_positions, key=candidate_positions.__getitem__) == sorted(
        baseline_positions, key=baseline_positions.__getitem__), "Spock transport changes definition order"

    residual = text
    for address, signature in SPOCK_OWNERS.items():
        marker = f"// ORIGINAL: 0x{address:08X}"
        start = residual.index(marker)
        end = _function_end(residual, residual.index(signature, start))
        residual = residual[:start] + residual[end:]
    code = [line.strip() for line in _without_comments(residual).splitlines() if line.strip()]
    allowed = [
        '#include "stdafx.h"', '#include "netdaemon.h"',
        "extern int SpockNotifyPending;", "extern int SpockHelpActive;",
        "extern int UnkGlobal0093A934;", "extern int ConsoleControlTurnActive;",
    ]
    assert code == allowed, "Spock TU contains code/state outside its includes, declarations and two mapped bodies"
    assert "ORIGINAL:" not in residual and "RECOVERY_BINDING:" not in residual
    assert not re.search(r"(?m)^\s*//\s*(?:symbol|body|compile_context)\b", residual)
    rows = read_text(text, path)
    assert {row.address for row in rows} == set(SPOCK_OWNERS)
    assert len(rows) == len(SPOCK_OWNERS)
    return blocks, baseline_positions


def _interface_block_start(text, marker_start):
    """Include the complete Purpose comment when the identity is inside it."""
    comment_start = text.rfind("/*", 0, marker_start)
    comment_end = text.rfind("*/", 0, marker_start)
    return comment_start if comment_start > comment_end else marker_start


def _interface_blocks(root, interface_text=None):
    """Validate all 15 transported complete envelopes against immutable parent."""
    root = Path(root)
    path = root / "src/netdaemon_interface.cpp"
    text = path.read_text() if interface_text is None else interface_text
    baseline = _interface_baseline(root)
    blocks = {}
    baseline_positions = {}
    baseline_ends = {}
    candidate_positions = {}

    for address, signature in INTERFACE_OWNERS.items():
        marker = f"// ORIGINAL: 0x{address:08X}"
        assert text.count(marker) == 1, f"interface identity {address:08X} must have one actual owner"
        assert baseline.count(marker) == 1, f"immutable parent identity {address:08X} missing/ambiguous"
        start = _interface_block_start(text, text.index(marker))
        end = _function_end(text, text.index(signature, start))
        block = text[start:end]
        base_start = _interface_block_start(baseline, baseline.index(marker))
        base_end = _function_end(baseline, baseline.index(signature, base_start))
        expected = baseline[base_start:base_end]
        assert block == expected, f"interface {address:08X} complete annotation/Purpose/body differs from immutable parent"
        blocks[address] = block
        baseline_positions[address] = base_start
        baseline_ends[address] = base_end
        candidate_positions[address] = start

    assert sorted(candidate_positions, key=candidate_positions.__getitem__) == sorted(
        baseline_positions, key=baseline_positions.__getitem__), "interface transport changes definition order"

    residual = text
    for address, signature in INTERFACE_OWNERS.items():
        marker = f"// ORIGINAL: 0x{address:08X}"
        start = _interface_block_start(residual, residual.index(marker))
        end = _function_end(residual, residual.index(signature, start))
        residual = residual[:start] + residual[end:]
    code = [line.strip() for line in _without_comments(residual).splitlines() if line.strip()]
    allowed = ['#include "stdafx.h"', '#include "netdaemon.h"', '#include "game.h"']
    assert code == allowed, "interface TU contains code outside its three includes and 15 mapped bodies"
    assert "ORIGINAL:" not in residual and "RECOVERY_BINDING:" not in residual
    assert not re.search(r"(?m)^\s*//\s*(?:symbol|body|compile_context)\b", residual)
    rows = read_text(text, path)
    assert {row.address for row in rows} == set(INTERFACE_OWNERS)
    assert len(rows) == len(INTERFACE_OWNERS)
    return blocks, baseline_positions, baseline_ends


def apply_interface_owner_comment_migration(source, reverse=False):
    """Project the exact parent9c2 interface comment block to/from current."""
    old = """// NetDaemon::synch itself is not recovered - 4,905 bytes with its own call
// targets, so it remains an original dependency. The fourteen synch_*
// forwarders below all funnel into it, loading the same daemon at 0x0093CD90
// that NetDaemonNet already binds above, and differ only in the opcode (and,
// for synch_diplo, which of the two leading arguments carries which value).

void __cdecl synch_veh(int id);
void __cdecl synch_base(int id);
void __cdecl synch_energy(int id);
void __cdecl synch_researching(int id);
void __cdecl synch_leader(int id);
void __cdecl synch_ai(int id);
void __cdecl synch_research(int id);
void __cdecl synch_alloc(int id);
void __cdecl synch_soc(int id);
void __cdecl synch_proto(int id);
void __cdecl synch_obs(int id);
void __cdecl synch_diplo(int a, int b);
void __cdecl synch_template(int id);
void __cdecl synch_radius(int id);

void __cdecl wait_2();"""
    new = """// Application-facing operations, implemented in netdaemon_interface.cpp.
// The fourteen notifications call the real NetDaemon::synch implementation
// with distinct opcodes and payload contracts; synch_diplo supplies two IDs.
// wait_2 selects the network pump or Win's ordinary wait using multiplayer state.

void __cdecl synch_veh(int id);
void __cdecl synch_base(int id);
void __cdecl synch_energy(int id);
void __cdecl synch_researching(int id);
void __cdecl synch_leader(int id);
void __cdecl synch_ai(int id);
void __cdecl synch_research(int id);
void __cdecl synch_alloc(int id);
void __cdecl synch_soc(int id);
void __cdecl synch_proto(int id);
void __cdecl synch_obs(int id);
void __cdecl synch_diplo(int a, int b);
void __cdecl synch_template(int id);
void __cdecl synch_radius(int id);

void __cdecl wait_2();"""
    before, after = (new, old) if reverse else (old, new)
    assert source.count(before) == 1, "exact parent/current interface comment block must be unique"
    source = source.replace(before, after, 1)
    old_wait = "void __cdecl wait_task();     // 0x0051F200"
    new_wait = "void __cdecl wait_task();     // 0x005FC700, Win's application event pump"
    before_wait, after_wait = (new_wait, old_wait) if reverse else (old_wait, new_wait)
    assert source.count(before_wait) == 1, "exact parent/current wait_task address comment must be unique"
    return source.replace(before_wait, after_wait, 1)


def historical_source(root, *, receiver_text=None, sender_text=None, header_text=None,
                      spock_text=None, interface_text=None):
    """Inverse only this migration, validating all discarded new-owner envelope."""
    root = Path(root)
    receiver = (root / "src/netdaemon.cpp").read_text() if receiver_text is None else receiver_text
    interface_blocks, interface_positions, interface_ends = _interface_blocks(root, interface_text)
    for signature in INTERFACE_OWNERS.values():
        assert signature not in receiver, f"interface definition must not remain in receiver: {signature}"
    for address, anchor in INTERFACE_INSERTIONS.items():
        assert receiver.count(anchor) == 1, f"sole interface insertion anchor for {address:08X}"
    interface_parent = _interface_baseline(root)
    wait_block = interface_blocks[0x0051F2C0]
    wait_end = interface_ends[0x0051F2C0]
    wait_anchor = interface_parent.index(INTERFACE_INSERTIONS[0x0051F2C0])
    wait_separator = interface_parent[wait_end:wait_anchor]
    assert wait_separator.strip() == "", "wait_2 has only a bounded whitespace separator before parent context"
    receiver = receiver.replace(INTERFACE_INSERTIONS[0x0051F2C0],
                                wait_block + wait_separator + INTERFACE_INSERTIONS[0x0051F2C0], 1)
    ordered_interface = sorted((address for address in interface_blocks if address != 0x0051F2C0),
                               key=interface_positions.__getitem__)
    pieces = []
    for index, address in enumerate(ordered_interface):
        pieces.append(interface_blocks[address])
        if index + 1 < len(ordered_interface):
            next_address = ordered_interface[index + 1]
            separator = interface_parent[interface_ends[address]:interface_positions[next_address]]
            assert separator.strip() == "", "adjacent interface records have only bounded whitespace between them"
            pieces.append(separator)
    last_interface = ordered_interface[-1]
    tail_separator = interface_parent[interface_ends[last_interface]:
                                      interface_parent.index(INTERFACE_INSERTIONS[0x00593220])]
    assert tail_separator.strip() == "", "interface family has only a bounded whitespace separator before parent context"
    pieces.append(tail_separator)
    restored_interface = "".join(pieces)
    receiver = receiver.replace(INTERFACE_INSERTIONS[0x00593220],
                                restored_interface + INTERFACE_INSERTIONS[0x00593220], 1)
    # Apply the new interface inverse first. Older packet and Spock projections
    # then operate on the receiver after its original interface owners exist.
    record = sender_record_block(root, header_text)
    sender = sender_block(root, sender_text)
    spock_blocks, positions = _spock_blocks(root, spock_text)
    assert receiver.count(INCLUDE) == 1, "sole record include expansion"
    assert SIGNATURE not in receiver, "sender must not remain in receiver TU"
    assert RECORD_START not in receiver, "record must not remain in receiver TU"
    assert receiver.count(NEXT) == 1, "sole historical sender insertion anchor"
    receiver = receiver.replace(INCLUDE + "\n\n", record, 1)
    assert INCLUDE not in receiver, "record include newline contract"
    receiver = receiver.replace(NEXT, sender + NEXT, 1)
    assert receiver.count(SPOCK_INSERTION) == 1, "sole original Spock insertion anchor"
    ordered = sorted(spock_blocks, key=positions.__getitem__)
    restored_spock = "\n\n".join(spock_blocks[address] for address in ordered) + "\n\n"
    return receiver.replace(SPOCK_INSERTION, restored_spock + SPOCK_INSERTION, 1)
