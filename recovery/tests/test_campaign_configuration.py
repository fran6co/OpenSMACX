"""Project campaign authorization controls, not proof of live recovery."""

import json
from pathlib import Path

import pytest

from match_recovery.campaign import _policy


ROOT = Path(__file__).resolve().parents[2]


def configured(tmp_path, monkeypatch):
    # The policy validates provisioned directory identities, not compiler health.
    pool = tmp_path / "compiler-pool"
    for name in ("worker-0", "worker-1"):
        (pool / name).mkdir(parents=True)
    monkeypatch.setenv("OPENSMACX_COMPILER_POOL", str(pool))
    return json.loads((ROOT / "recovery/campaign.json").read_text())


def test_campaign_authorizes_two_isolated_workers_and_verified_publication(tmp_path, monkeypatch):
    policy = configured(tmp_path, monkeypatch)
    profiles = _policy(ROOT, policy)
    assert policy["max_parallel"] == len(profiles["resources"]["slots"]) == 2
    assert policy["max_batches"] == 1
    assert policy["destination_branch"] == "refs/heads/master"
    assert profiles["driver"]["acceptance_scope"] == "change"
    assert profiles["checkpoint"]["allow_migration"] is False
    assert policy["publication"]["allow_migration"] is False
    assert policy["publication"]["enabled"] is True
    assert policy["publication"]["cleanup"] is True
    reservation, = profiles["planner"]["reserved_worktrees"]
    assert reservation["path"] == "build/recovery/control-integration-native"
    assert reservation["tasks"] == ["control-sprite-defaults"]
    assert reservation["evidence"]


def test_campaign_refuses_more_workers_than_declared_resources(tmp_path, monkeypatch):
    policy = configured(tmp_path, monkeypatch)
    policy["max_parallel"] = 3
    with pytest.raises(ValueError, match="exceeds provisioned compiler capacity"):
        _policy(ROOT, policy)
