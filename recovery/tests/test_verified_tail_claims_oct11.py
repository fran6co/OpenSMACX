"""Two production-recorded obligations; not full reference/game acceptance."""
from dataclasses import fields, replace
import json
from pathlib import Path
import subprocess

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "84df17bd71a4f8da80608096518756fa5f424033"
FIXTURE = Path(__file__).with_name("verified-tail-claims-oct11.patch")
RECORDED = {0x618E20, 0x4B9E90}


def apply_verified_tail_claims(source, relative):
    section = next(part for part in FIXTURE.read_text().split("diff --git ")[1:]
                   if part.startswith("a/" + relative + " b/" + relative + "\n"))
    for hunk in section.split("\n@@ ")[1:]:
        lines = hunk.splitlines(keepends=True)[1:]
        before = "".join(line[1:] for line in lines if line.startswith((" ", "-")))
        after = "".join(line[1:] for line in lines if line.startswith((" ", "+")))
        assert before and source.count(before) == 1
        source = source.replace(before, after, 1)
    return source


def expected_recorded_tail(record):
    return replace(record, byte_exact=True) if record.address in RECORDED else record


@pytest.mark.parametrize("relative", ["src/caviar.cpp", "src/statuswin.cpp"])
def test_only_two_claim_additions_preserve_complete_source(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    current = path.read_bytes()
    before = read_text(old.decode(), path)
    after = read_text(current.decode(), path)
    selected = [expected_recorded_tail(row) for row in before if row.address in RECORDED]
    assert selected and all(not row.byte_exact for row in before if row.address in RECORDED)
    assert write_bytes(old, selected) == current
    assert apply_verified_tail_claims(old.decode(), relative) == current.decode()
    assert [row.address for row in before] == [row.address for row in after]
    for original, recorded in zip(before, after):
        expected = expected_recorded_tail(original)
        for field in fields(original):
            if field.name == "region":
                # FILE records carry the entire TU, including the two edited
                # comments. The exact writer/fixture comparisons above check it.
                continue
            assert getattr(expected, field.name) == getattr(recorded, field.name)
    if relative == "src/caviar.cpp":
        assert not next(row for row in after if row.address == 0x617DB0).byte_exact


def test_exact_native_authorization():
    from match_recovery.tasks import Task
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/verified-tail-claims-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert {subject.address for subject in task.subjects} == RECORDED
    assert all(subject.address_space == "pe:va" and subject.image_sha256 ==
               "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
               for subject in task.subjects)
