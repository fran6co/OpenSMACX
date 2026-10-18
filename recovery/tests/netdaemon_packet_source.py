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


def historical_source(root, *, receiver_text=None, sender_text=None, header_text=None):
    """Inverse only this migration, validating all discarded new-owner envelope."""
    root = Path(root)
    receiver = (root / "src/netdaemon.cpp").read_text() if receiver_text is None else receiver_text
    record = sender_record_block(root, header_text)
    sender = sender_block(root, sender_text)
    assert receiver.count(INCLUDE) == 1, "sole record include expansion"
    assert SIGNATURE not in receiver, "sender must not remain in receiver TU"
    assert RECORD_START not in receiver, "record must not remain in receiver TU"
    assert receiver.count(NEXT) == 1, "sole historical sender insertion anchor"
    receiver = receiver.replace(INCLUDE + "\n\n", record, 1)
    assert INCLUDE not in receiver, "record include newline contract"
    return receiver.replace(NEXT, sender + NEXT, 1)
