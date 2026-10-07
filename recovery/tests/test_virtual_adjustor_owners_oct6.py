"""Preservation controls for source-owned RadioButton/Dialogs virtual adjustors.

These are source/history controls, not binary or full-game acceptance.
"""
from functools import lru_cache
from pathlib import Path
import subprocess

import pytest
from match_recovery import binding_source
from match_recovery.source import Mode, read_file, read_text
from test_mapwin_adjustor_owner_history_oct6 import MAPWIN_MIGRATED_ADDRESSES

ROOT = Path(__file__).resolve().parents[2]
BASE = "ab27be18e1bcb016d2e2a675eb474da187081e93"
SPECS = (
    (0x00406FF0, "src/radiobutton.cpp", "?on_redraw@RadioButton@@$4PPPPPPPM@A@AEXXZ"),
    (0x00407000, "src/radiobutton.cpp", "?on_mouse_move@RadioButton@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x00407020, "src/radiobutton.cpp", "?on_left_down@RadioButton@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x00407040, "src/radiobutton.cpp", "?on_left_double_click@RadioButton@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x00407170, "src/dialogs.cpp", "?on_redraw@Dialogs@@$4PPPPPPPM@A@AEXXZ"),
    (0x00407190, "src/dialogs.cpp", "?on_mouse_move@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x004071A0, "src/dialogs.cpp", "?on_mouse_leave@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x004071C0, "src/dialogs.cpp", "?on_left_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x004071D0, "src/dialogs.cpp", "?on_left_up@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x004071F0, "src/dialogs.cpp", "?on_right_down@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x00407220, "src/dialogs.cpp", "?on_left_double_click@Dialogs@@$4PPPPPPPM@A@AEXHH@Z"),
    (0x00407260, "src/dialogs.cpp", "?on_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXH@Z"),
    (0x00407270, "src/dialogs.cpp", "?pass_dialog_focus@Dialogs@@$4PPPPPPPM@A@AEXXZ"),
    (0x004072C0, "src/dialogs.cpp", "?on_redraw@Dialogs@@$4PPPPPPPM@KBI@AEXXZ"),
)

# These five names are evidence-backed virtual API corrections made after
# BASE=ab27be1; exact expected symbols remain asserted rather than waiving names.
# Exact follow-on cohort moved after the original virtual-owner tests were authored.
FOLLOW_ON_MIGRATED_ADDRESSES = frozenset({
    0x0051DE60, 0x0051DED0, 0x0051DF40,
    0x0060CF60, 0x0060CFB0, 0x0060FE80,
    0x00611850, 0x00611880, 0x006118B0, 0x006127D0,
})

CURRENT_CALLBACK_SYMBOLS = {
    0x00612CC0: "?on_key_down@Dialogs@@UAEHH@Z",
    0x00612E80: "?on_left_click@Dialogs@@UAEXHH@Z",
    0x00612F60: "?on_right_click@Dialogs@@UAEXHH@Z",
    0x00612FC0: "?on_mousewheel@Dialogs@@UAEXH@Z",
    0x0060E020: "?on_key_down@RadioButton@@UAEHH@Z",
}

@lru_cache(None)
def before(path):
    # Use the full known commit identity rather than the current branch tip.
    content = subprocess.check_output(["git", "show", BASE + ":" + path], cwd=ROOT)
    return {r.address: r for r in read_text(content.decode(), ROOT / path)}

@pytest.mark.parametrize("address,owner,symbol", SPECS)
def test_generated_owner_retains_original_identity_and_history(address, owner, symbol):
    old = before("src/adjustor_thunks.cpp")[address]
    rows = read_file(ROOT / owner)
    selected = [r for r in rows if r.address == address]
    assert len(selected) == 1
    row = selected[0]
    assert row.mode is Mode.FILE
    assert row.symbol == symbol
    assert (row.name, row.image_spans, row.byte_exact, row.semantic, row.kind,
            row.levers, row.ruled_out, row.unrecoverable, row.deferred) == (
        old.name, old.image_spans, old.byte_exact, old.semantic, old.kind,
        old.levers, old.ruled_out, old.unrecoverable, old.deferred)
    evidence = binding_source.parse((ROOT / owner).read_text(), rows)[row.line]
    assert symbol in "\n".join(evidence["evidence"])
    assert "constructor" in "\n".join(evidence["evidence"]).lower()

def current_source_records():
    result = {}
    for path in (ROOT / "src").rglob("*.cpp"):
        for row in read_file(path):
            result.setdefault(row.address, []).append((path, row))
    return result


@pytest.mark.parametrize("path", ("src/dialogs.cpp", "src/radiobutton.cpp"))
def test_unmigrated_bodies_keep_actual_dialogs_and_radio_extraction(path):
    # Check this lifecycle migration against its fixed pre-change checkpoint.
    # BASE remains the original seed-identity reference in the other controls;
    # intervening reviewed migrations legitimately changed symbols/extraction.
    raw = subprocess.check_output([
        "git", "show", "55195bdb7:" + path], cwd=ROOT).decode()
    old = {row.address: row for row in read_text(raw, ROOT / path)}
    moved = ({address for address, _, _ in SPECS} | MAPWIN_MIGRATED_ADDRESSES
             | FOLLOW_ON_MIGRATED_ADDRESSES
             | {0x406910, 0x407100, 0x4070F0, 0x407280, 0x4072A0})
    current = {row.address: row for row in read_file(ROOT / path)}
    for address, prior in old.items():
        if address in moved:
            continue
        row = current[address]
        assert (row.name, row.image_spans, row.byte_exact,
                row.semantic, row.mode, row.extract_error) == (
            prior.name, prior.image_spans, prior.byte_exact,
            prior.semantic, prior.mode, prior.extract_error)
        assert row.symbol == prior.symbol
        if prior.mode is Mode.BODY:
            assert row.body == prior.body


def test_retired_collection_preserves_all_unmigrated_source_identities():
    assert not (ROOT / "src/adjustor_thunks.cpp").exists()
    assert not (ROOT / "src/adjustor_thunks.h").exists()
    old = before("src/adjustor_thunks.cpp")
    moved = ({address for address, _, _ in SPECS} | MAPWIN_MIGRATED_ADDRESSES
             | FOLLOW_ON_MIGRATED_ADDRESSES)
    current = current_source_records()
    for address, prior in old.items():
        if address in moved:
            continue
        hits = current.get(address, [])
        assert len(hits) == 1, f"{address:08X} lost or duplicated after owner transfer"
        _path, row = hits[0]
        # Original identities/claims survive, while the explicit adapter body
        # and its extraction mode legitimately become a generated owner entry.
        assert (row.name, row.image_spans, row.byte_exact, row.semantic, row.kind) == (
            prior.name, prior.image_spans, prior.byte_exact, prior.semantic, prior.kind)
        assert row.symbol and "adjust_" not in row.symbol
        if prior.mode is Mode.BODY:
            assert row.body == prior.body
