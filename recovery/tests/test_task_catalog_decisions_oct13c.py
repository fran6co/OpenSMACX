"""The real project catalogue must not admit manual cards as native tasks."""
import json
from pathlib import Path
import subprocess

from match_recovery.tasks import Task, TaskStore


ROOT = Path(__file__).resolve().parents[2]
BASELINE = 'a9d43ab7b9175e1c78b4df967bb068eb05798151'


def test_current_catalog_is_loadable_by_native_task_consumers():
    store = TaskStore(ROOT)
    files = sorted((ROOT / 'recovery/tasks').glob('*.json'))
    assert files
    for path in files:
        task = store.read(path.stem)
        assert task.task_id == path.stem


def test_prior_manual_decisions_remain_exactly_preserved():
    names = subprocess.check_output(
        ['git', 'ls-tree', '-r', '--name-only', BASELINE, '--', 'recovery/tasks'],
        cwd=ROOT, text=True).splitlines()
    moved = []
    for name in names:
        if not name.endswith('.json'):
            continue
        original = subprocess.check_output(['git', 'show', BASELINE + ':' + name], cwd=ROOT)
        try:
            Task.from_dict(json.loads(original))
        except (ValueError, KeyError, TypeError):
            destination = ROOT / 'recovery/decisions' / Path(name).name
            assert destination.read_bytes() == original
            assert not (ROOT / name).exists()
            moved.append(destination)
    assert moved, 'control needs historical non-native decisions'
