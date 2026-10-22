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


@lru_cache(maxsize=None)
def _baseline(root):
    return subprocess.check_output(["git", "show", BASE + ":src/netdaemon.cpp"],
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


def historical_source(root, *, receiver_text=None, sender_text=None, header_text=None,
                      spock_text=None):
    """Inverse only this migration, validating all discarded new-owner envelope."""
    root = Path(root)
    receiver = (root / "src/netdaemon.cpp").read_text() if receiver_text is None else receiver_text
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
