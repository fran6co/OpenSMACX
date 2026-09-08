"""Synthetic debugger artifacts are hints, never recovery acceptance evidence."""

import json
import shutil
import struct
import subprocess

import pytest

from match_recovery import debug_codeview as cv, debug_sidecar as debug
from match_recovery.cli import main
from test_binary import pe_fixture
from test_codeview import coff, record


def type_object(path, records, section=b".debug$T"):
    path.write_bytes(coff(struct.pack("<I", 2) + records, section=section))
    return cv.Object(path)


def pch(tmp_path):
    definition = record(cv.LF_MODIFIER, struct.pack("<IH", 0x74, 1))
    path = tmp_path / "arbitrary-name.obj"
    type_object(path, definition + record(0x14, struct.pack("<I", 123)), b".debug$P")
    return cv.precompiled(path)


def test_precompiled_type_context_uses_signature_range_not_filename_or_adjacency(tmp_path):
    provider = pch(tmp_path)
    imported = record(0x100E, struct.pack("<III", 0x1000, 1, 123) + b"\x05a.pch")
    pointer = record(cv.LF_POINTER, struct.pack("<II", 0x1000, 0xA))
    obj = type_object(tmp_path / "caller.obj", imported + pointer)
    table = obj.types([provider])
    assert table[0x1000]["kind"] == "modifier"
    assert table[0x1001] == {"kind": "pointer", "target": 0x1000, "size": 4}


def test_provider_end_marker_retains_local_index_but_is_not_exported(tmp_path):
    modifier = record(cv.LF_MODIFIER, struct.pack("<IH", 0x74, 1))
    pointer = record(cv.LF_POINTER, struct.pack("<II", 0x1000, 0xA))
    obj = type_object(tmp_path / "provider.obj", modifier + record(0x14, struct.pack("<I", 123)) + pointer,
                      b".debug$P")
    table, imported, exported = obj.type_table()
    assert imported is None and exported["count"] == 1
    assert set(exported["types"]) == {0x1000}
    assert set(table) == {0x1000, 0x1002} and table[0x1002]["kind"] == "pointer"


@pytest.mark.parametrize("damage", ["missing", "signature", "count", "ambiguous"])
def test_missing_or_conflicting_precompiled_type_provider_refuses(tmp_path, damage):
    provider = pch(tmp_path)
    imported = record(0x100E, struct.pack("<III", 0x1000, 1, 123) + b"\x05a.pch")
    obj = type_object(tmp_path / "caller.obj", imported)
    providers = [provider]
    if damage == "missing": providers = []
    elif damage == "ambiguous": providers *= 2
    else: provider[damage] += 1
    with pytest.raises(ValueError, match="matching signature and index range"):
        obj.types(providers)


@pytest.mark.parametrize("body", [b"\x00\x80", b"\x01\x80\x00", b"\x88\x88"])
def test_bad_numeric_leaf_never_becomes_zero(body):
    with pytest.raises(ValueError):
        cv.numeric(body, 0)


def test_truncated_name_refuses():
    with pytest.raises(ValueError):
        cv.pascal(b"\x05xy", 0)


def test_unknown_field_tail_is_not_a_partial_complete_class(tmp_path):
    first = struct.pack("<HHIH", 0x1405, 0, 0x74, 4) + b"\x04slot"
    obj = type_object(tmp_path / "fields.obj", record(cv.LF_FIELDLIST, first + b"\x77\x77"))
    assert obj.types() == {}
    assert "unsupported field leaf" in obj.diagnostics[0]


def test_forward_type_remains_a_declaration_not_a_zero_sized_definition():
    unit = debug.Die("compile_unit")
    types = debug.Types({0x1000: {"kind": "struct", "name": "Forward", "forward": True}}, unit)
    die = types.die(0x1000)
    assert ("declaration", "flag", 1) in die.attributes
    assert not any(name == "byte_size" for name, _, _ in die.attributes)


def test_modifier_cycles_terminate_and_do_not_invent_array_bounds():
    unit = debug.Die("compile_unit")
    types = debug.Types({0x1000: {"kind": "modifier", "target": 0x1000}}, unit)
    assert types.size(0x1000) == 0 and types.die(0x1000) is None
    with pytest.raises(ValueError, match="negative"):
        debug.uleb(-1)


def test_debug_locals_require_an_explicit_frame_base():
    procedure = {"name": "f", "external": True, "type": 0,
                 "locals": [{"name": "value", "type": 0x74, "offset": -4}]}
    for frame, expected in (("none", 0), ("ebp", 1)):
        unit = debug.Die("compile_unit")
        assert debug._subprogram(unit, debug.Types({}, unit), procedure, 0x401000, 0x401001,
                                 frame_base=frame) == expected


@pytest.fixture
def artifact(tmp_path):
    image = bytearray(pe_fixture(b"\xc3\x90\xc3"))
    struct.pack_into("<I", image, 0x98 + 16, 0x1000)
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    exe = tmp_path / "sample.exe"
    exe.write_bytes(image)
    map_path = tmp_path / "sample.map"
    map_path.write_text(" Publics by Value\n"
                        " 0001:00000000 _f 00401000 f first.obj\n"
                        " 0001:00000000 _alias 00401000 f first.obj\n"
                        " 0001:00000002 _next 00401002 f second.obj\n"
                        "\nLine numbers for first.obj(q:\\sources\\sample.cpp) segment .text\n"
                        " 12 0001:00000000\n")
    objects = tmp_path / "objects"
    objects.mkdir()
    return exe, map_path, tmp_path / "debug.sym", tmp_path, objects


def test_cli_needs_only_supplied_build_artifacts_not_project_configuration(artifact, capsys):
    arguments = ["debug-sidecar", "--summary", "--path-map", "q:/sources/=/code/"]
    for key, value in zip(("exe", "map", "out", "comp-dir", "build-root"), artifact):
        arguments.extend(["--" + key, str(value)])
    assert main(arguments) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["artifact_written"] and report["symbols"] == 3
    assert report["verification"] == "debug_hints_only" and not report["ready_for_collection"]
    assert report["debug_coverage"] == "incomplete" and report["diagnostic_count"] > 0
    data = artifact[2].read_bytes()
    assert data[:4] == b"\x7fELF" and b"/code/sample.cpp\0" in data
    assert struct.unpack_from("<I", data, 24)[0] == 0x401000


def test_source_drives_are_not_implicitly_mapped_to_host_root(artifact):
    _, lines = debug.read_map(artifact[1])
    assert lines["first.obj"][0][0] == "q:/sources/sample.cpp"


@pytest.mark.parametrize("damage", ["image", "map", "object", "destination"])
def test_changed_inputs_or_destination_refuse_publication(artifact, monkeypatch, damage):
    exe, map_path, out, _, objects = artifact
    out.write_bytes(b"existing sidecar")
    actual = debug.render
    def change(*args, **kwargs):
        result = actual(*args, **kwargs)
        target = {"image": exe, "map": map_path, "object": objects / "new.obj", "destination": out}[damage]
        target.write_bytes(target.read_bytes() + b"changed" if target.exists() else b"new object")
        return result
    monkeypatch.setattr(debug, "render", change)
    with pytest.raises(ValueError, match="changed"):
        debug.build(*artifact)
    assert out.read_bytes() == (b"existing sidecarchanged" if damage == "destination" else b"existing sidecar")


def test_duplicate_object_basenames_are_not_silently_first_wins(artifact):
    objects = artifact[-1]
    (objects / "nested").mkdir()
    for path in (objects / "first.obj", objects / "nested/first.obj"):
        path.write_bytes(b"object")
    with pytest.raises(ValueError, match="ambiguous debug object"):
        debug.build(*artifact)


def test_output_alias_cannot_overwrite_an_input(artifact):
    artifact[2].symlink_to(artifact[0])
    original = artifact[0].read_bytes()
    with pytest.raises(ValueError, match="non-aliased"):
        debug.build(*artifact)
    assert artifact[0].read_bytes() == original


def test_malformed_primary_input_keeps_previous_output(artifact):
    artifact[2].write_bytes(b"previous")
    artifact[0].write_bytes(b"bad PE")
    with pytest.raises(ValueError):
        debug.build(*artifact)
    assert artifact[2].read_bytes() == b"previous"


@pytest.mark.skipif(not shutil.which("readelf"), reason="optional independent ELF/DWARF reader")
def test_independent_reader_accepts_synthetic_sidecar(artifact):
    debug.build(*artifact)
    result = subprocess.run(["readelf", "--debug-dump=info", "--debug-dump=decodedline", str(artifact[2])],
                            capture_output=True, text=True, timeout=20)
    assert result.returncode == 0 and not result.stderr
    assert "sample.cpp" in result.stdout and "DW_TAG_subprogram" in result.stdout
