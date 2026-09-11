"""Discovery routing requires an explicit bound coordinator policy transition."""

from copy import deepcopy

import pytest

from match_recovery.campaign_choices import coordinator_authority
from match_recovery.model import digest
from test_campaign_authority import authority
from test_preservation import project


@pytest.mark.parametrize("flag", [True, False, 1, "true", None])
def test_bound_route_opt_in_is_the_only_new_coordinator_permission(authority, flag):
    root, directory, config, original, _, token = authority
    current = deepcopy(original)
    current["coordinator"]["route_discoveries"] = flag
    token = {**token, "coordinator": deepcopy(current["coordinator"])}
    before = {path.name: path.read_bytes() for path in directory.iterdir()}
    if flag is True:
        assert coordinator_authority(root, directory, config, current, token, publish=True) == digest(current)
        assert coordinator_authority(root, directory, config, current, token, publish=True) == digest(current)
        assert len(list(directory.glob("coordinator-authorization-*.json"))) == 1
    else:
        with pytest.raises(ValueError): coordinator_authority(root, directory, config, current, token, publish=True)
        assert not list(directory.glob("coordinator-authorization-*.json"))
    assert {name: (directory / name).read_bytes() for name in before} == before


def test_routing_opt_in_does_not_allow_command_or_provider_policy_change(authority):
    root, directory, config, original, _, token = authority
    current = deepcopy(original)
    current["coordinator"]["route_discoveries"] = True
    current["coordinator"]["command"]["timeout"] += 1
    token = {**token, "coordinator": current["coordinator"]}
    with pytest.raises(ValueError): coordinator_authority(root, directory, config, current, token, publish=True)
    assert not list(directory.glob("coordinator-authorization-*.json"))
