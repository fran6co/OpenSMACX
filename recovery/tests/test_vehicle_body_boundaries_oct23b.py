"""Mapped-body ownership, original identity, and seed-history controls.

This reads the original task-base annotations and the current source through
the native source parser. It complements C++ host behavior controls; it is not
native ABI, build/link, or game-runtime acceptance.
"""
from pathlib import Path
import json
import re
import subprocess

from match_recovery.source import read_file, read_text
from test_game_logic_seed_batch_behavior import _definition


ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/veh.cpp"
BASE = "1a5f2853fc3ef6eaeb00cbe71d0f358cf354b1b3"
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ENTRIES = (
    (0x005678C0, 0x0056A89E, "?air_power@@YAHHHHPAH0@Z",
     "int __cdecl air_power("),
    (0x005B6260, 0x005B8AE2, "?spot_all@@YAXHH@Z",
     "void __cdecl spot_all("),
)


def _baseline():
    return subprocess.check_output(
        ["git", "show", f"{BASE}:src/veh.cpp"], cwd=ROOT, timeout=30
    ).decode("utf-8")


def _origin(row):
    if row.origin is None:
        return None
    return tuple((item.kind, item.key, item.prose,
                  tuple(line for _, line in item.lines)) for item in row.origin)


def _entry_body(source, address, signature):
    rows = {row.address: row for row in read_text(source, SOURCE)}
    assert address in rows, hex(address)
    row = rows[address]
    assert row.region and "BODY GOES HERE." not in row.region
    return row, _definition(row.region, signature)


def test_full_algorithms_own_their_mapped_boundaries_and_keep_history():
    baseline = _baseline()
    current = SOURCE.read_text(encoding="utf-8")
    task = json.loads((ROOT / "recovery/tasks/vehicle-body-boundaries-oct23b.json")
                      .read_text(encoding="utf-8"))
    authorized = {(row["address"], row["image_sha256"])
                  for row in task["subjects"]}
    assert all((address, PIN) in authorized for address, *_ in ENTRIES)
    old_rows = {row.address: row for row in read_text(baseline, SOURCE)}
    new_rows = {row.address: row for row in read_file(SOURCE)}
    assert len(ENTRIES) == 2

    for address, end, symbol, signature in ENTRIES:
        assert address in old_rows and address in new_rows
        old, new = old_rows[address], new_rows[address]
        assert old.image_spans == ((address, end),)
        assert new.image_spans == old.image_spans
        # The native source parser exposes these original mangled names in
        # `name`; `symbol` is intentionally empty in both source annotations.
        assert old.name == new.name == symbol
        assert new.symbol == old.symbol
        assert new.path == SOURCE
        assert new.region and "BODY GOES HERE." not in new.region
        for name in ("address", "mode", "name", "image_spans", "symbol", "kind",
                     "exclusion", "recipe", "levers", "ruled_out", "unrecoverable",
                     "deferred"):
            assert getattr(new, name) == getattr(old, name), (hex(address), name)
        assert _origin(new) == _origin(old)
        for name in ("byte_exact", "semantic"):
            if getattr(old, name):
                assert getattr(new, name), f"lost positive {name} claim at {address:#x}"
        body = _definition(new.region, signature)
        assert len(body) > 1000, (hex(address), len(body))
        assert body.count("{") == body.count("}"), hex(address)

    _, air = _entry_body(current, ENTRIES[0][0], ENTRIES[0][3])
    _, spot = _entry_body(current, ENTRIES[1][0], ENTRIES[1][3])

    # These original call boundaries must remain visible in the actual mapped
    # entries. Helpers may be called only where they are real game services;
    # recovery-only decompositions must not replace mapped-owner control flow.
    air_services = ("speed", "base_find", "stack_check", "battle_fight",
                    "compute_odds", "veh_lift", "veh_drop", "action_home")
    spot_services = ("spot_tile", "wake_stack",
                     "want_to_wake", "encounter", "has_abil", "is_sensor",
                     "whose_territory", "add_goal", "draw_radius", "interlude")
    for service in air_services:
        assert re.search(r"\b" + re.escape(service) + r"\s*\(", air), service
    for service in spot_services:
        assert re.search(r"\b" + re.escape(service) + r"\s*\(", spot), service

    assert not re.search(r"\bair_power_[A-Za-z0-9_]+\s*\(", air), (
        "air_power must own its current recovery-only dispatch/helper graph")
    for stale in ("SpotAllSourceContext", "inspect_spot_all_source(",
                  "may_mark_spot_all_map_dirty(", "scan_spot_all_"):
        assert stale not in spot, stale

