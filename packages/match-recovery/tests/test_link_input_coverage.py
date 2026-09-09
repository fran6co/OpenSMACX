"""Conservative input populations; no common-size or selected-import claims."""

import hashlib
import json

import pytest

from match_recovery.link_input_coverage import input_coverage
from test_binary import coff_fixture
from test_coff_inventory import common_object, imported
from test_link_file_evidence import stamp
from test_toolchain_components import archive


def inputs(root):
    library, obj, exp, tool = [root / name for name in ("runtime.lib", "app.obj", "app.exp", "linker")]
    library.write_bytes(archive([("runtime.obj/", common_object()), ("DEMO.dll/", imported()),
                                ("DEMO.dll/", imported("_Other@8"))]))
    obj.write_bytes(coff_fixture())
    exp.write_bytes(coff_fixture())
    tool.write_bytes(b"test tool")
    log = ("Start Pass1\n    Searching " + str(library) + ":\n"
           "Start Generating .exp file\nStart Pass1\nEnd Pass1\nEnd Generating .exp file\nEnd Pass1\n"
           "Start Pass2\napp.exp\napp.obj\nRUNTIME.lib(runtime.obj)\n"
           "RUNTIME.lib(DEMO.dll)\nRUNTIME.lib(DEMO.dll)\nEnd Pass2\n")
    value = {"schema_version": 2, "link_log_sha256": hashlib.sha256(log.encode()).hexdigest(),
             "cwd": str(root), "tool": str(tool), "arguments": [str(obj)], "explicit_inputs": [str(obj)],
             "files": [{"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())}
                       for path in (library, obj, tool)],
             "library_searches": [{"spelling": str(library), "path": str(library)}],
             "generated_exports": [{"path": str(exp), "existed_before": False, "after": stamp(exp.read_bytes())}],
             "capture_gaps": []}
    return value, log


def verify(value, log):
    return input_coverage(json.dumps(value).encode(), log.encode())


def test_all_final_labels_have_conservative_candidates_without_selecting_repeated_imports(tmp_path):
    report = verify(*inputs(tmp_path))
    assert report["result"] == "pass" and report["input_count"] == 5 and report["record_count"] == 5
    assert report["format_counts"] == {"coff": 3, "short_import": 2}
    assert report["inputs"][-1]["candidate_count"] == report["inputs"][-2]["candidate_count"] == 2
    assert report["inputs"][-1]["selection"] == "unknown"
    assert report["allocation_verification"] == "unknown" and not report["ready_for_collection"]
    assert report["common_requests"] == [{"path": str(tmp_path / "runtime.lib"), "member": "runtime.obj",
                                          "occurrence": 0, "symbol": "_g", "size": 4}]


@pytest.mark.parametrize("damage", ["existing", "missing", "bytes", "no_generation", "duplicate_label", "symlink"])
def test_unproved_generated_export_never_closes_input_coverage(tmp_path, damage):
    value, log = inputs(tmp_path)
    row = value["generated_exports"][0]
    if damage == "existing":
        row["existed_before"] = True
    elif damage == "missing":
        value["generated_exports"] = []
    elif damage == "bytes":
        (tmp_path / "app.exp").write_bytes(b"changed output")
    elif damage == "no_generation":
        log = log.replace("Start Generating .exp file\nStart Pass1\nEnd Pass1\nEnd Generating .exp file\n", "")
    elif damage == "duplicate_label":
        log = log.replace("app.exp\n", "app.exp\napp.exp\n")
    else:
        (tmp_path / "app.exp").rename(tmp_path / "same")
        (tmp_path / "app.exp").symlink_to(tmp_path / "same")
    value["link_log_sha256"] = hashlib.sha256(log.encode()).hexdigest()
    assert verify(value, log)["result"] == "unknown"


def test_even_an_unselected_unsupported_archive_record_prevents_absence_evidence(tmp_path):
    value, log = inputs(tmp_path)
    path = tmp_path / "runtime.lib"
    path.write_bytes(archive([("runtime.obj/", common_object()), ("DEMO.dll/", imported()), ("unused.obj/", b"unsupported")]))
    value["files"][0]["before"] = value["files"][0]["after"] = stamp(path.read_bytes())
    report = verify(value, log)
    assert report["result"] == "unknown" and report["unsupported_records"][0]["member"] == "unused.obj"
    assert report["common_requests"]


def test_unlisted_explicit_common_object_is_still_in_the_candidate_population(tmp_path):
    value, log = inputs(tmp_path)
    path = tmp_path / "common-only.obj"
    path.write_bytes(common_object(32))
    value["explicit_inputs"].append(str(path))
    value["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    report = verify(value, log)
    assert report["result"] == "pass"
    assert any(row["path"] == str(path) and row["size"] == 32 for row in report["common_requests"])


def test_same_basename_explicit_objects_are_not_silently_selected(tmp_path):
    value, log = inputs(tmp_path)
    directory = tmp_path / "other"
    directory.mkdir()
    path = directory / "app.obj"
    path.write_bytes(coff_fixture())
    value["explicit_inputs"].append(str(path))
    value["files"].append({"path": str(path), "before": stamp(path.read_bytes()), "after": stamp(path.read_bytes())})
    report = verify(value, log)
    assert report["result"] == "unknown" and report["inputs"][1]["candidate_count"] == 2


def test_unknown_final_input_cannot_be_filled_from_neighbors(tmp_path):
    value, log = inputs(tmp_path)
    log = log.replace("app.obj\n", "missing.obj\n")
    value["link_log_sha256"] = hashlib.sha256(log.encode()).hexdigest()
    assert verify(value, log)["result"] == "unknown"
