"""Wine search-path translation and pre/post compiler input capture."""

import json

import pytest

from match_recovery.link_file_evidence import verify_link_files
from recovery.environment.wine_link import capture_before, capture_after, host_path


def setup(tmp_path):
    sdk = tmp_path / "SDK"
    sdk.mkdir()
    tool = sdk / "LINK.EXE"
    tool.write_bytes(b"synthetic tool")
    (sdk / "RUNTIME.LIB").write_bytes(b"synthetic library")
    (tmp_path / "app.obj").write_bytes(b"synthetic object")
    return sdk, tool


def test_capture_resolves_actual_wine_case_and_preserves_file_observations(tmp_path):
    sdk, tool = setup(tmp_path)
    before = capture_before(["app.obj", "runtime.lib", "/OUT:app.exe", "/VERBOSE"], tool, [str(sdk)], tmp_path)
    searched = "Z:" + str(sdk / "runtime.lib").replace("/", "\\")
    log = "Start Pass1\n    Searching " + searched + ":\nEnd Pass1\nStart Pass2\napp.obj\nRUNTIME.lib(runtime.obj)\nEnd Pass2\n"
    after = capture_after(before, log)
    assert not after["capture_gaps"]
    assert after["library_searches"] == [{"spelling": searched, "path": str(sdk / "RUNTIME.LIB")}]
    assert after["explicit_inputs"] == [str(tmp_path / "app.obj")]
    assert verify_link_files(json.dumps(after).encode(), log.encode())["result"] == "pass"


def test_searched_library_created_after_capture_cannot_gain_a_prelink_identity(tmp_path):
    sdk, tool = setup(tmp_path)
    before = capture_before(["app.obj"], tool, [str(sdk)], tmp_path)
    new = sdk / "NEW.LIB"
    new.write_bytes(b"late input")
    log = "Start Pass1\n    Searching " + str(new) + ":\nEnd Pass1\nStart Pass2\napp.obj\nEnd Pass2\n"
    after = capture_after(before, log)
    assert next(row for row in after["files"] if row["path"] == str(new))["before"] is None
    assert verify_link_files(json.dumps(after).encode(), log.encode())["result"] == "unknown"


@pytest.mark.parametrize("damage", ["collision", "symlink", "drive"])
def test_ambiguous_or_unsupported_wine_path_never_selects_a_file(tmp_path, damage):
    sdk, tool = setup(tmp_path)
    path = str(sdk / "runtime.lib")
    if damage == "collision":
        (sdk / "runtime.lib").write_bytes(b"different input")
    elif damage == "symlink":
        (sdk / "RUNTIME.LIB").unlink()
        (sdk / "RUNTIME.LIB").symlink_to(tool)
    else:
        path = "C:\\runtime.lib"
    with pytest.raises(ValueError):
        host_path(path, tmp_path)


def test_response_files_and_unavailable_search_roots_remain_capture_gaps(tmp_path):
    sdk, tool = setup(tmp_path)
    before = capture_before(["@missing.rsp"], tool, [str(tmp_path / "missing")], tmp_path)
    assert len(before["gaps"]) == 2


def test_explicit_library_path_outside_search_roots_is_captured(tmp_path):
    sdk, tool = setup(tmp_path)
    library = sdk / "RUNTIME.LIB"
    before = capture_before([str(library)], tool, [], tmp_path)
    assert str(library) in before["before"] and before["explicit_inputs"] == [str(library)]


@pytest.mark.parametrize("existing", [False, True])
def test_export_output_records_real_prelink_existence_not_an_invented_before_hash(tmp_path, existing):
    sdk, tool = setup(tmp_path)
    path = tmp_path / "app.exp"
    if existing:
        path.write_bytes(b"previous output")
    before = capture_before(["app.obj", "/implib:app.lib"], tool, [str(sdk)], tmp_path)
    assert before["generated_exports"] == [{"path": str(path), "existed_before": existing}]
    path.write_bytes(b"new export object")
    after = capture_after(before, "")
    assert after["schema_version"] == 2
    assert after["generated_exports"][0]["existed_before"] is existing
    assert after["generated_exports"][0]["after"]["size"] == len(b"new export object")
