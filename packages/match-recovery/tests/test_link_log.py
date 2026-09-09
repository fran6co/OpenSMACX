"""Synthetic final-input observations, not compiler-installation attestation."""

import pytest

from match_recovery.msvc_link_log import parse_link_log


LOG = """Start Pass1
Searching Libraries
    Searching Z:\\compiler\\RUNTIME.lib:
      Found _f
        Loaded RUNTIME.lib(runtime.obj)
Done Searching Libraries
Start Generating .exp file
Library manager banner and arguments
Start Pass1
    Searching Z:\\compiler\\OTHER.lib:
End Pass1
End Generating .exp file
End Pass1
    Discarded _unused from RUNTIME.lib(runtime.obj)
Start Pass2
     app.exp
     application.obj
     RUNTIME.lib(runtime.obj)
     DEMO.lib(DEMO.dll)
     DEMO.lib(DEMO.dll)
End Pass2
"""


def test_final_inputs_preserve_duplicates_and_separate_nested_library_searches():
    report = parse_link_log(LOG.encode())
    assert report["result"] == "pass" and report["input_count"] == 5
    assert report["inputs"][0] == {"label": "app.exp", "library": None, "member": "app.exp"}
    assert report["inputs"][2] == {"label": "RUNTIME.lib(runtime.obj)",
                                    "library": "RUNTIME.lib", "member": "runtime.obj"}
    assert report["inputs"][3] == report["inputs"][4]
    assert report["searched_libraries"] == [
        {"path": "Z:\\compiler\\RUNTIME.lib", "phase": "executable"},
        {"path": "Z:\\compiler\\OTHER.lib", "phase": "export_library"}]
    assert report["input_identity"] == report["allocation_verification"] == "unknown"
    assert not report["ready_for_collection"]


@pytest.mark.parametrize("text", ["", "Searching Libraries\nDone Searching Libraries\n",
    "Start Pass1\nEnd Pass1\n", "Start Pass2\napp.obj\nEnd Pass2\n",
    LOG.replace("End Pass2\n", ""), LOG + LOG,
    LOG.replace("Start Pass2", "Start Pass3"),
    LOG.replace("End Generating .exp file\n", ""),
    LOG.replace("End Pass1\nEnd Generating", "End Generating"),
    LOG.replace("     app.exp", "     unexplained output"),
    LOG.replace("     app.exp", "     runtime.lib(nested(member))"),
    LOG.replace("     app.exp", "     demo.lib( padded.obj)"),
    LOG.replace("     app.exp", "     bad\x7fname.obj"),
    LOG.replace("     app.exp", "     bad\x01name.obj"),
    LOG.replace("     app.exp", "     café.obj"),
    LOG + "Searching unknown.lib:\n",
    "Start Pass1\nEnd Pass1\nStart Pass2\nEnd Pass2\n"])
def test_incomplete_concatenated_or_unsupported_phases_and_rows_refuse(text):
    with pytest.raises(ValueError):
        parse_link_log(text.encode())


def test_import_descriptor_control_symbol_is_not_a_filename():
    assert parse_link_log(LOG.replace("Found _f", "Found \x7fDEMO_NULL_THUNK_DATA").encode())["result"] == "pass"


def test_no_export_generation_is_required_for_a_plain_executable():
    report = parse_link_log(b"Start Pass1\nEnd Pass1\nStart Pass2\na.obj\nEnd Pass2\n")
    assert report["export_generations"] == 0 and report["input_count"] == 1
