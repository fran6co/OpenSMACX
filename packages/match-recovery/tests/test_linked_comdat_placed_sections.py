"""Positive placement queries annotate retention without any selection waiver."""

import pytest

from match_recovery.linked_comdat_selection import comdat_selection, _entry
from match_recovery.linked_context import linked_build
from test_linked_comdat_private_placements import fixture as private_fixture
from test_linked_comdat_unique_placements import fixture


@pytest.mark.parametrize("fpo", [False, True])
def test_placed_returns_the_exact_external_placement_certificate(tmp_path, fpo):
    context = fixture(tmp_path / "project", fpo=fpo)
    loser_section, peer_section = (2, 3) if fpo else (1, 2)
    with linked_build(context) as native, comdat_selection(native) as selected:
        # The domain always asks removal first; the ANY query also derives the
        # peer placement this certificate must agree with.
        assert selected.loser("dependency.obj", loser_section) is not None
        placement = selected.placed("dependency.obj", peer_section)
        assert placement["disposition"] == "independently_placed_nodup_root"
        assert placement["root_identity_kind"] == "external"
        assert placement["queried_section"] == peer_section and peer_section in placement["sections"]
        recorded, = selected.unique_placements()
        assert placement["placement_id"] == recorded["placement_id"]


def test_placed_refuses_an_unqualified_placement_and_caches_it(tmp_path):
    context = fixture(tmp_path / "project", damage="overlap")
    with linked_build(context) as native, comdat_selection(native) as selected:
        with pytest.raises(ValueError):
            selected.placed("dependency.obj", 2)
        # The refusal is deterministic per physical query, not a re-derivation.
        with pytest.raises(ValueError):
            selected.placed("dependency.obj", 2)
        state = _entry(selected)
        assert state["unique_refusals"]


def test_placed_distinguishes_any_winner_from_loser_copy(tmp_path):
    context = fixture(tmp_path / "project")
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 1) is not None
        # The certified ANY winner is positively selected.
        winner = selected.placed("unit0.obj", 1)
        assert winner["disposition"] == "selected_any_winner"
        assert winner["root_identity_kind"] == "external"
        assert winner["queried_section"] == 1 and 1 in winner["sections"]
        certificate, = [row for row in selected.certificates()
                        if row["certificate_id"] == winner["certificate_id"]]
        assert certificate["winner"]["input"] == native.inputs.objects.resolve("unit0.obj")["identity"]
        # The loser copy is a removal question, never a placement.
        assert selected.placed("dependency.obj", 1) is None


@pytest.mark.parametrize("two", [False, True])
def test_placed_covers_each_private_peer_by_owner(tmp_path, two):
    context = private_fixture(tmp_path / "project", two=two)
    owners = ("dependency.obj", "unit0.obj") if two else ("dependency.obj",)
    with linked_build(context) as native, comdat_selection(native) as selected:
        assert selected.loser("dependency.obj", 1) is not None
        placements = {row["owner"]: row for row in selected.unique_placements()}
        assert set(placements) == set(owners)
        for owner in owners:
            placement = selected.placed(owner, 2)
            assert placement["disposition"] == "independently_placed_nodup_root"
            assert placement["root_identity_kind"] == "private"
            assert placement["queried_section"] == 2 and 2 in placement["sections"]
            assert placement["placement_id"] == placements[owner]["placement_id"]
