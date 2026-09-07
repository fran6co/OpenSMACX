"""Foreign type dependencies cannot establish standalone layout evidence."""

import pytest

from match_recovery.application_layout import verify_layout
from match_recovery.codeview import ClassLayouts, class_layout
from match_recovery.codeview_data import data_layout
from match_recovery.layout_check import check_layout
from match_recovery.model import Origin
from match_recovery.pe_coff import Coff
from match_recovery.tasks import Task, TaskStore
from test_codeview import coff, debug_types, record
from test_layout_check import layout_project
from test_object_data import data_fixture
from test_symbol_check import project


EXTERNAL_TYPES = (0x0013, 0x0014, 0x0016, 0x100E, 0x1501, 0x1509, 0x1515)


@pytest.mark.parametrize("leaf", EXTERNAL_TYPES, ids=hex)
def test_external_type_dependencies_remain_refused_for_data(leaf):
    with pytest.raises(ValueError, match="standalone.*PCH or type servers"):
        data_layout(Coff(data_fixture(types=record(leaf, b""))), "_g")


@pytest.mark.parametrize("leaf", EXTERNAL_TYPES, ids=hex)
@pytest.mark.parametrize("entry", ["single", "index"])
def test_external_type_dependencies_are_refused_even_with_a_complete_local_class(leaf, entry):
    obj = Coff(coff(debug_types() + record(leaf, b"")))
    with pytest.raises(ValueError, match="standalone.*PCH or type servers"):
        if entry == "single":
            class_layout(obj, "Widget")
        else:
            ClassLayouts(obj)


@pytest.mark.parametrize("leaf", EXTERNAL_TYPES, ids=hex)
def test_external_type_dependencies_keep_product_layout_coverage_unknown(layout_project, leaf):
    report = check_layout(layout_project, compiler=lambda *_: coff(debug_types() + record(leaf, b"")))
    assert report["result"] == "unknown" and report["counts"] == {"unknown": 1}
    assert not report["observations"]
    assert all(row["observation"] == "unmeasurable" for row in report["coverage"])
    assert all("PCH or type servers" in row["reason"] for row in report["errors"])


@pytest.mark.parametrize("leaf", EXTERNAL_TYPES, ids=hex)
def test_external_type_dependencies_keep_task_layout_coverage_unknown(layout_project, leaf):
    TaskStore(layout_project).create(Task("owner", "example", "coordinator", "Recover ownership", "b" * 40,
                                          (Origin("a" * 64, "pe:va", 0x401000),),
                                          ("code/f.cc", "code/types.h"), ("synthetic",)))
    report = verify_layout(layout_project, "owner", compiler=lambda *_: coff(debug_types() + record(leaf, b"")))
    assert report["result"] == "unknown"
    assert all(row["result"] == "unknown" for row in report["members"])
    assert all(row["observation"] == "unmeasurable" for row in report["coverage"])
