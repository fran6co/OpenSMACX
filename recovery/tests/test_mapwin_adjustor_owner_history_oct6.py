"""Preserve identities/history for the constructor-installed MapWin-view cohort.

Source/history evidence only; no standalone/native compilation, linked selection,
or game acceptance is claimed. Expected class-view facts were transcribed from
fresh-mapwin-cohort/manifest.json (packet 95d55fd9cf6d31fdaa39ec92c7252e7f009d5f15a5d7635e731d3574e2cc1e7f).
"""
from functools import lru_cache
from pathlib import Path
import subprocess

import pytest
from match_recovery import binding_source
from match_recovery.source import Mode, read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
MIGRATION_BASE = "8fa917225f83aba858ab6a87e31675dab717c802"
MIGRATION_SOURCE_SHA256 = "91c2d74854425f48cc8f0b2820a059d30cc1c730001975008f14e8ed0616d14d"


@lru_cache(None)
def before_adjustors():
    raw = subprocess.check_output(
        ["git", "show", f"{MIGRATION_BASE}:src/adjustor_thunks.cpp"], cwd=ROOT
    )
    # This guard ties the test's migration base to the exact pre-transfer source
    # measured and hashed in the cohort manifest.
    import hashlib
    assert hashlib.sha256(raw).hexdigest() == MIGRATION_SOURCE_SHA256
    return {row.address: row for row in read_text(raw.decode(), ROOT / "src/adjustor_thunks.cpp")}


_ADAPTER_OWNERS = ("map_win1", "map_win4", "map_win5", "plan_win1")
_CALLBACKS = (
    "redraw", "sys_close", "left_click", "left_down", "left_up",
    "right_click", "right_down", "left_double_click", "button_clicked",
    "nc_hittest",
)


def _is_migrated_adapter(symbol):
    return any(
        symbol.startswith(f"?adjust_{owner}_on_{callback}@@")
        for owner in _ADAPTER_OWNERS for callback in _CALLBACKS
    )


# Shared by the old remaining-row tests. The cohort migration base is committed
# and predates only these transfers; unrelated rows remain subject to their old
# exact-set/history assertions.
MAPWIN_MIGRATED_ADDRESSES = frozenset(
    address for address, row in before_adjustors().items()
    if _is_migrated_adapter(row.symbol)
)


METHODS = {
    "map_redraw": (0x0046B1D0, "?on_redraw@MapWin@@QAEXXZ", 0x30),
    "plan_redraw": (0x0048AF30, "?on_redraw@PlanWin@@QAEXXZ", 0x30),
    "sys_close": (0x0046F880, "?on_sys_close@MapWin@@QAEXXZ", 0x38),
    "left_click": (0x0046EBA0, "?on_left_click@MapWin@@QAEXHH@Z", 0x4C),
    "left_down": (0x0046ED30, "?on_left_down@MapWin@@QAEXHH@Z", 0x50),
    "left_up": (0x0046EFF0, "?on_left_up@MapWin@@QAEXHH@Z", 0x54),
    "right_click": (0x0046EBE0, "?on_right_click@MapWin@@QAEXHH@Z", 0x58),
    "right_down": (0x0046EC10, "?on_right_down@MapWin@@QAEXHH@Z", 0x5C),
    "left_double_click": (0x0046EBD0, "?on_left_double_click@MapWin@@QAEXHH@Z", 0x70),
    "button_clicked": (0x0046F8C0, "?on_button_clicked@MapWin@@QAEXH@Z", 0xB0),
    "nc_hittest": (0x0046F660, "?on_nc_hittest@MapWin@@QAEXHH@Z", 0x12C),
}

VIEWS = (
    {
        "name": "MapWin", "owner": "src/mapwin.cpp", "tag": "A",
        "table": "??_7MapWin@@6BWin@@@", "table_va": 0x0066A57C,
        "constructor": "??0MapWin@@QAE@H@Z", "constructor_va": 0x004626E0,
            "methods": ("map_redraw", "sys_close", "left_click", "left_down", "left_up",
                    "right_click", "right_down", "left_double_click", "button_clicked",
                    "nc_hittest"),
    },
    {
        "name": "PlanWin", "owner": "src/planwin.cpp", "tag": "FOE",
        "table": "??_7PlanWin@@6BWin@@@", "table_va": 0x0066D2AC,
        "constructor": "??0PlanWin@@QAE@H@Z", "constructor_va": 0x0048BCD0,
            "methods": ("plan_redraw", "sys_close", "left_click", "left_down", "left_up",
                    "right_click", "right_down", "left_double_click", "button_clicked",
                    "nc_hittest"),
    },
    {
        "name": "Console Win view", "owner": "src/mapwin.cpp", "tag": "CDCI",
        "table": "??_7Console@@6BWin@@@", "table_va": 0x0066EC18,
        "constructor": "??0Console@@QAE@H@Z", "constructor_va": 0x0050F460,
        "methods": ("map_redraw", "left_click", "left_down", "left_up", "right_click",
                    "right_down", "left_double_click", "button_clicked"),
    },
)


def emitted_symbol(target_symbol, tag, method):
    owner_and_name, tail = target_symbol.split("@@", 1)
    assert tail.startswith("Q")
    tail = tail[1:]
    # The native view emits the int-returning non-client hit-test slot, while
    # the old direct target catalogued the void helper prototype.
    if method == "nc_hittest":
        tail = tail.replace("X", "H", 1)
    return f"{owner_and_name}@@$4PPPPPPPM@{tag}@{tail}"


def test_cohort_addresses_match_the_28_manifest_rows():
    assert len(MAPWIN_MIGRATED_ADDRESSES) == 28
    all_current = {
        row.address: row
        for owner in ("src/mapwin.cpp", "src/planwin.cpp")
        for row in read_file(ROOT / owner)
    }
    found = set()
    for view in VIEWS:
        rows = read_file(ROOT / view["owner"])
        for method in view["methods"]:
            target_va, target_symbol, _slot = METHODS[method]
            # PlanWin's own redraw entry is a direct primary-view thunk; the
            # inherited MapWin callbacks use the secondary FOE adjustment.
            tag = "A" if view["name"] == "PlanWin" and method == "plan_redraw" else view["tag"]
            expected = emitted_symbol(target_symbol, tag, method)
            matches = [row for row in rows if row.symbol == expected]
            assert len(matches) == 1, (view["name"], expected)
            row, = matches
            assert row.address in MAPWIN_MIGRATED_ADDRESSES
            assert row.address not in found
            found.add(row.address)
            assert row.mode is Mode.FILE
            assert row.symbol == expected
            evidence = binding_source.parse((ROOT / view["owner"]).read_text(), rows)[row.line]["evidence"]
            evidence_text = "\n".join(evidence)
            target_va = f"{target_va:08X}"
            assert view["constructor"] in evidence_text
            assert f"{view['constructor_va']:08X}" in evidence_text
            assert f"{view['table_va']:08X}" in evidence_text
            assert f"Pinned view slot +0x{METHODS[method][2]:X} contains original thunk VA {row.address:08X}" in evidence_text
            assert f"tail reaches {target_va} ({target_symbol})" in evidence_text
    assert found == MAPWIN_MIGRATED_ADDRESSES
    assert set(all_current) >= found


@pytest.mark.parametrize("view", VIEWS, ids=lambda row: row["name"])
def test_migrated_rows_retain_original_identity_claims_and_history(view):
    old = before_adjustors()
    rows = read_file(ROOT / view["owner"])
    remaining = {row.address for row in read_file(ROOT / "src/adjustor_thunks.cpp")}
    for method in view["methods"]:
        target_va, target_symbol, _slot = METHODS[method]
        tag = "A" if view["name"] == "PlanWin" and method == "plan_redraw" else view["tag"]
        symbol = emitted_symbol(target_symbol, tag, method)
        current, = (row for row in rows if row.symbol == symbol)
        prior = old[current.address]
        assert current.mode is Mode.FILE
        assert current.address not in remaining
        assert _is_migrated_adapter(prior.symbol)
        for field in ("name", "image_spans", "byte_exact", "semantic", "kind",
                      "levers", "ruled_out", "unrecoverable", "deferred"):
            assert getattr(current, field) == getattr(prior, field), (hex(current.address), field)
        assert current.address not in {
            row.address for row in read_file(ROOT / "src/adjustor_thunks.cpp")
        }
