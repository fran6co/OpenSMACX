"""Emitted shared-view renames retain origins, spans and evidence exactly."""
from dataclasses import replace

import pytest

from match_recovery.shared_source import parse_shared, write_shared_symbol

PIN = "a" * 64
PATH = "src/views.h"
SOURCE = ("// RECOVERY_SHARED: v1 0x00401000 0x00401003 ?old@View@@UAEPAXXZ 0x00402000\n"
          "virtual void *old() { return 0; }\n"
          "// RECOVERY_SHARED: v1 0x00401000 0x00401003 ?other@View@@UAEPAXXZ 0x00402000\n"
          "virtual void *other() { return 0; }\n")
NEW = "?named@View@@UAEPAXXZ"


def apply(raw, expected=None, binding=None, symbol=NEW):
    rows = parse_shared(raw.decode(), PATH, PIN, address_space="pe:va")
    return write_shared_symbol(raw, expected=rows if expected is None else expected,
                               binding=rows[0] if binding is None else binding, symbol=symbol)


@pytest.mark.parametrize("newline", ("\n", "\r\n", "\r"))
def test_symbol_only_change_preserves_source_bytes_and_other_views(newline):
    raw = SOURCE.replace("\n", newline).encode()
    assert apply(raw) == raw.replace(b"?old@View@@UAEPAXXZ", NEW.encode(), 1)


def test_refuses_stale_or_incomplete_snapshot():
    raw = SOURCE.encode()
    rows = parse_shared(SOURCE, PATH, PIN, address_space="pe:va")
    for expected in (rows[:1], tuple(replace(row, span=replace(row.span, end=0x401004)) for row in rows)):
        with pytest.raises(ValueError, match="snapshot"):
            apply(raw, expected=expected)
    with pytest.raises(ValueError, match="snapshot"):
        apply(raw, binding=replace(rows[0], line=2))


@pytest.mark.parametrize("symbol", ("", "a b", "bad\nmarker", "x/*y", "x//y", "x\\y", "x??/y", "é"))
def test_refuses_marker_injection(symbol):
    with pytest.raises(ValueError, match="annotation-safe"):
        apply(SOURCE.encode(), symbol=symbol)


def test_refuses_duplicate_symbol():
    with pytest.raises(ValueError, match="duplicate"):
        apply(SOURCE.encode(), symbol="?other@View@@UAEPAXXZ")


def test_refuses_macro_context_and_prior_splice_but_allows_later_macro():
    with pytest.raises(ValueError, match="macro|#define"):
        apply(("#define V \\\n" + SOURCE).encode())
    with pytest.raises(ValueError, match="spliced"):
        apply(("// continued \\\n" + SOURCE).encode())
    raw = (SOURCE + "#define V \\\n+ 1\n").encode()
    assert apply(raw) == raw.replace(b"?old@View@@UAEPAXXZ", NEW.encode(), 1)
