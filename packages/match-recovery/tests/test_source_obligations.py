"""Object target aliases do not rewrite historical initializer interpretation."""

from match_recovery.source_obligations import task_obligations
from test_source_history import commit, task_project
from test_task_source import project
from test_source_configuration import configured


def test_direct_target_rebinding_retains_one_original_interpretation(task_project):
    path = task_project / "code/owner.cc"
    path.write_text(path.read_text() + "\n// RECOVERY_OBJECT: v2 0x00000020 0x00000028 _old 0x00000010\n")
    commit(task_project, "Add bounded original object interpretation")
    before = task_obligations(task_project, "owner")
    old, = before["interpretations"]
    path.write_text(path.read_text().replace(
        "RECOVERY_OBJECT: v2 0x00000020 0x00000028 _old 0x00000010",
        "RECOVERY_OBJECT: v4 0x00000020 0x00000028 _old 0x00000010 target _current"))
    after = task_obligations(task_project, "owner")
    assert len(after["interpretations"]) == 1
    current, = after["interpretations"]
    assert current["interpretation_id"] == old["interpretation_id"]
    assert current["entity_id"] == old["entity_id"]
    assert current["current"]
    assert current["value"]["symbol"] == "_old"
    assert current["value"].get("target_symbol") is None
    assert [row["obligation_id"] for row in before["obligations"]] == [
        row["obligation_id"] for row in after["obligations"]]
