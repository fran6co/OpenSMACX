"""Causal source-body attribution controls using synthetic COFF only."""

import base64
from dataclasses import replace
import re
from types import SimpleNamespace

import pytest

from match_recovery.body_attribution import (BodyAttribution, _definition_edit,
                                             _renamed_symbol)
from match_recovery.pe_coff import Coff
from test_function_witness import object_bytes


UNIT = "src/owner.cpp"
HEADER = "src/model.h"
TARGET = "_sample"
BASE = object_bytes([(TARGET, b"\xc3", ()), ("_other", b"\xc3", ())])


def _snapshot(root, header, *, owners=1):
    (root / "src").mkdir(exist_ok=True)
    marker_rows = []
    records = []
    for index in range(owners):
        line = index * 2 + 1
        address = 0x401000 + index * 0x10
        marker_rows.extend((
            f"// ORIGINAL: 0x{address:08X} {TARGET} 0x{address:08X}-0x{address + 6:08X}\n",
            f"// body {HEADER}\n",
        ))
        records.append({"entity_id": f"entity-{index}", "path": UNIT,
                        "line": line, "body_path": HEADER, "context_path": UNIT})
    unit_text = "".join(marker_rows)
    (root / UNIT).write_text(unit_text)
    (root / HEADER).write_text(header)
    files = {}
    for path, content in ((UNIT, unit_text.encode()), (HEADER, header.encode())):
        files[path] = {"content_base64": base64.b64encode(content).decode("ascii")}
    return {"files": files, "source_configuration": {"symbol_format": "none"},
            "records": records}


class FakeOverlay:
    def __init__(self, root, snapshot, *, mode="success"):
        self.root, self.snapshot, self.mode = root, snapshot, mode
        self.calls = []
        self.original_header = base64.b64decode(snapshot["files"][HEADER]["content_base64"])

    def overlay(self, source, replacements, arguments, flags):
        self.calls.append((source, dict(replacements), arguments, flags))
        header = replacements[HEADER]
        if header == self.original_header:
            if self.mode == "unchanged-drift":
                return object_bytes([(TARGET, b"\xcc", ()), ("_other", b"\xc3", ())])
            return BASE
        if self.mode == "compile-error":
            raise ValueError("synthetic overlay compile failure")
        text = header.decode("utf-8")
        probe_match = re.search(r"match_recovery_body_probe_[0-9a-f]{24}", text)
        if probe_match is None:
            return object_bytes([("_other", b"\xc3", ())])
        renamed = "_" + probe_match.group(0)
        functions = [("_other", b"\xc3", ())]
        if self.mode in {"wrong-namespace", "wrong-overload", "inline-wrapper-retains-target"}:
            functions.append((TARGET, b"\xc3", ()))
        if self.mode != "no-renamed-definition":
            functions.append((renamed, b"\xc3", ()))
        return object_bytes(functions)


def _qualify(root, snapshot, compiler=None, *, header=None, record_index=0):
    attribution = BodyAttribution(root, snapshot)
    record = attribution.record(f"entity-{record_index}")
    result = attribution.qualify(record, Coff(BASE), unit=UNIT,
                                 arguments=("configured-argument",),
                                 flags=("configured-flag",),
                                 compiler=compiler or FakeOverlay(root, snapshot))
    return attribution, result


def test_definition_rename_preserves_msvc_abi_suffix_and_signature_arity():
    symbol = "?sample@@YAHH@Z"
    renamed = _renamed_symbol(symbol, "sample", "match_recovery_body_probe_0123456789abcdef01234567")
    assert renamed == "?match_recovery_body_probe_0123456789abcdef01234567@@YAHH@Z"
    assert renamed[len("?match_recovery_body_probe_0123456789abcdef01234567@@"):] == "YAHH@Z"
    assert _renamed_symbol("_sample@8", "sample", "probe") == "_probe@8"
    with pytest.raises(ValueError, match="ordinary free-function"):
        _renamed_symbol("?sample@Widget@@QAEHXZ", "Widget::sample", "probe")


def test_definition_edit_balances_multiline_default_argument_lambda_braces():
    source = ("inline int sample(\n"
              "    int value = []() { return 3; }()\n"
              ") {\n"
              "    return value;\n"
              "}\n")
    candidate = SimpleNamespace(line=1, end_line=5)
    start, end, edited = _definition_edit(source, candidate, "sample", "fresh_probe")
    assert start == 0 and end == len(source)
    assert edited.startswith("inline int sample(\n    int value = []() { return 3; }()\n);\n")
    assert "inline int fresh_probe(\n    int value = []() { return 3; }()\n) {" in edited


def test_exact_body_rename_qualifies_with_unchanged_overlay_control(tmp_path):
    header = "inline int sample() { return 1; }\n"
    snapshot = _snapshot(tmp_path, header)
    compiler = FakeOverlay(tmp_path, snapshot)
    attribution, result = _qualify(tmp_path, snapshot, compiler)
    assert result["result"] == "pass"
    assert result["path"] == HEADER and result["symbol"] == TARGET
    assert result["evidence"].startswith("unchanged-overlay equivalence")
    assert attribution.compile_attempts == 2 and len(compiler.calls) == 2
    assert compiler.calls[0][1][HEADER] == header.encode()
    assert compiler.calls[1][1][HEADER] != header.encode()
    assert compiler.calls[0][2:] == (("configured-argument",), ("configured-flag",))


def test_requested_body_cannot_borrow_a_different_snapshot_mapping_probe(tmp_path):
    header = 'inline int sample() { return 1; }\n'
    snapshot = _snapshot(tmp_path, header)
    snapshot['files']['src/other.h'] = {'content_base64': base64.b64encode(header.encode()).decode()}
    attribution = BodyAttribution(tmp_path, snapshot)
    record = replace(attribution.record('entity-0'), body='src/other.h')
    with pytest.raises(ValueError, match='differs from its captured source mapping'):
        attribution.qualify(record, Coff(BASE), unit=UNIT, arguments=(), flags=(),
                            compiler=FakeOverlay(tmp_path, snapshot))


@pytest.mark.parametrize("mode", ["wrong-namespace", "wrong-overload"])
def test_wrong_namespace_or_overload_keeps_mapped_definition_and_refuses(tmp_path, mode):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode=mode)
    with pytest.raises(ValueError, match="did not remove the selected function definition"):
        _qualify(tmp_path, snapshot, compiler)


def test_inline_wrapper_false_positive_is_rejected_when_exact_root_remains(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="inline-wrapper-retains-target")
    with pytest.raises(ValueError, match="did not remove the selected function definition"):
        _qualify(tmp_path, snapshot, compiler)


def test_unchanged_overlay_object_drift_refuses_before_definition_edit(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="unchanged-drift")
    with pytest.raises(ValueError, match="unchanged source overlay differs"):
        _qualify(tmp_path, snapshot, compiler)
    assert len(compiler.calls) == 1


def test_unsupported_overlay_client_and_placeholder_refuse(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    class NoOverlay:
        pass
    with pytest.raises(ValueError, match="no source-overlay capability"):
        _qualify(tmp_path, snapshot, NoOverlay())

    placeholder = _snapshot(tmp_path, "int sample() {\n    // BODY GOES HERE.\n}\n")
    with pytest.raises(ValueError, match="placeholder"):
        _qualify(tmp_path, placeholder)


def test_overlapping_same_definition_owners_refuse_without_compiling(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n", owners=2)
    compiler = FakeOverlay(tmp_path, snapshot)
    with pytest.raises(ValueError, match="ranges overlap"):
        _qualify(tmp_path, snapshot, compiler)
    assert not compiler.calls


def test_renamed_definition_must_be_present_in_overlay(tmp_path):
    snapshot = _snapshot(tmp_path, "inline int sample() { return 1; }\n")
    compiler = FakeOverlay(tmp_path, snapshot, mode="no-renamed-definition")
    with pytest.raises(ValueError, match="expected one defined COFF function"):
        _qualify(tmp_path, snapshot, compiler)
