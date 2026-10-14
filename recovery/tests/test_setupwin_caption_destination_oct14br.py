"""Actual-source regression for SetupWin's local popup-caption buffer."""
from recovery.tests.test_setupwin_rightside_actual_source import actual_body, run_host


def test_actual_caption_destination_and_edge_cases(tmp_path):
    result = run_host(tmp_path, actual_body())
    assert result.returncode == 0, result.stdout + result.stderr


def test_filename_destination_mutant_compiles_then_fails_semantically(tmp_path):
    body = actual_body()
    correct = "strcat(token, caption);"
    mutant = "strcat(filename, caption);"
    assert correct in body
    result = run_host(tmp_path, body.replace(correct, mutant, 1))
    assert result.returncode == 97, result.stdout + result.stderr
    assert "SEMANTIC97: popup caption does not mutate filename argument" in result.stderr
