"""Fresh linked dependency code; synthetic data only, never target admission."""

import base64
from dataclasses import replace
import hashlib
import json
import struct

import pytest

from match_recovery import component_link
from match_recovery.cli import main
from match_recovery.component_source import render_component
from match_recovery.components import ToolchainManifest
from match_recovery.import_source import parse_imports
from match_recovery.model import Delivery, Span
from test_binary import coff_fixture, component_fixture
from test_build_check import project, write_config, command
from test_imports import DECLARATION, image_imports
from test_link_map import MAP, executable
from test_toolchain_components import archive


def inputs(code=None, raw_image=None, *, symbol_damage=None):
    image = raw_image or executable()
    obj = bytearray(code or coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20)))
    if symbol_damage:
        at = struct.unpack_from("<I", obj, 8)[0] + 18
        if symbol_damage == "common":
            struct.pack_into("<I", obj, at + 8, 4)
        elif symbol_damage == "absolute":
            struct.pack_into("<h", obj, at + 12, -1)
        else:
            obj[at + 16] = 105 if symbol_damage == "weak" else 3
    raw = archive([("runtime.obj/", bytes(obj))])
    old = component_fixture(image)
    binding = replace(old.bindings[0], unit="runtime.obj", spans=(Span(0x401000, 0x401006),))
    manifest = ToolchainManifest(replace(old.component, delivery=Delivery.TOOLCHAIN),
                                 hashlib.sha256(raw).hexdigest(), "Synthetic library", (binding,))
    return manifest, raw, image


def verify(manifest, raw, image, mapping=MAP, **kwargs):
    return component_link.verify_linked_toolchain(manifest, raw, image, mapping.encode(), library="RUNTIME", **kwargs)


def test_matching_linked_code_does_not_establish_original_or_data_acceptance():
    report = verify(*inputs())
    assert report["result"] == report["functions"][0]["references"] == "pass"
    assert report["functions"][0]["target_placements"] == [
        {"symbol": "_g", "address": 0x401020, "basis": "public_placement_only"}]
    assert report["original_verification"] == report["storage_verification"] == "unknown"
    assert not report["persisted"] and not report["ready_for_collection"]


@pytest.mark.parametrize("damage", ["opcode", "operand", "library", "member", "extent", "pin"])
def test_link_code_and_selection_disagreements_fail(damage):
    manifest, raw, image = inputs()
    mapping = MAP
    if damage in ("opcode", "operand"):
        changed = bytearray(image)
        changed[512 if damage == "opcode" else 513] ^= 1
        image = bytes(changed)
    elif damage in ("library", "member"):
        mapping = mapping.replace("RUNTIME:" if damage == "library" else "runtime.obj",
                                  "OTHER:" if damage == "library" else "wrong.obj")
    elif damage == "extent":
        manifest = replace(manifest, bindings=(replace(manifest.bindings[0], spans=(Span(0x401000, 0x401011),)),))
    else:
        raw += b"wrong distribution"
    report = verify(manifest, raw, image, mapping)
    assert report["result"] == "fail"
    if damage == "pin":
        assert report["member_reads"] == 0


@pytest.mark.parametrize("damage", ["weak", "static", "absolute"])
def test_unsupported_linker_or_local_targets_stay_unknown(damage):
    assert verify(*inputs(symbol_damage=damage))["result"] == "unknown"


def test_common_public_address_does_not_claim_allocation_extent_or_ownership():
    report = verify(*inputs(symbol_damage="common"))
    assert report["result"] == "pass" and report["storage_verification"] == "unknown"


@pytest.mark.parametrize("damage", [None, "missing", "duplicate", "wrong_library", "truncated"])
def test_supplied_final_input_observations_are_required_not_inferred_from_map(damage):
    from test_link_log import LOG
    log = LOG
    if damage == "missing":
        log = log.replace("     RUNTIME.lib(runtime.obj)\n", "")
    elif damage == "duplicate":
        log = log.replace("     RUNTIME.lib(runtime.obj)\n", "     RUNTIME.lib(runtime.obj)\n" * 2)
    elif damage == "wrong_library":
        log = log.replace("     RUNTIME.lib(runtime.obj)", "     OTHER.lib(runtime.obj)")
    elif damage == "truncated":
        with pytest.raises(ValueError):
            verify(*inputs(), link_log=log.replace("End Pass2", "").encode())
        return
    report = verify(*inputs(), link_log=log.encode())
    assert report["result"] == ("pass" if damage is None else "unknown")
    assert report["link_inputs"]["input_identity"] == "unknown"
    assert report["storage_verification"] == "unknown"


def configure_log(project, text):
    value = json.loads((project / "recovery/build.json").read_text())
    value["commands"][0]["argv"][2] += "; (p/'app.link.log').write_text(" + repr(text) + ")"
    if not any(item["format"] == "msvc-link-log" for item in value["outputs"]):
        value["outputs"].append({"path": "app.link.log", "format": "msvc-link-log"})
    write_config(project, **value)


def test_fresh_link_log_is_a_validated_artifact_consumed_by_native_verification(project, capsys):
    from test_link_log import LOG
    args = configure(project)
    configure_log(project, LOG)
    assert main(args) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["link_inputs"]["input_count"] == 5
    assert report["functions"][0]["final_input_observation"] == {"matches": 1, "result": "pass"}
    configure_log(project, "truncated")
    assert main(args) == 3
    damaged = json.loads(capsys.readouterr().out)
    assert damaged["fresh_build"]["outputs"][-1]["result"] == "unknown"


def test_fresh_link_log_cannot_change_after_binary_verification(project, capsys, monkeypatch):
    from test_link_log import LOG
    args = configure(project)
    configure_log(project, LOG)
    original = component_link.verify_linked_toolchain
    def mutate(*values, **kwargs):
        report = original(*values, **kwargs)
        next((project / "build/checks").glob("build-*/app.link.log")).write_text(LOG + "changed")
        return report
    monkeypatch.setattr(component_link, "verify_linked_toolchain", mutate)
    assert main(args) == 3
    assert "changed" in capsys.readouterr().err


@pytest.mark.parametrize("damage", [None, "other_path", "missing_before", "log_digest", "missing_log"])
def test_fresh_component_checks_physical_archive_evidence_when_configured(project, capsys, damage):
    from test_link_log import LOG
    from test_link_file_evidence import evidence
    args = configure(project)
    configure_log(project, LOG)
    receipt = evidence(project)
    if damage == "other_path":
        shadow = project / "shadow.lib"
        shadow.write_bytes((project / "runtime.lib").read_bytes())
        receipt["files"][1]["path"] = str(shadow)
        receipt["library_searches"][0]["path"] = str(shadow)
    elif damage == "missing_before":
        receipt["files"][1]["before"] = None
    elif damage == "log_digest":
        receipt["link_log_sha256"] = "0" * 64
    value = json.loads((project / "recovery/build.json").read_text())
    value["commands"][0]["argv"][2] += "; (p/'app.inputs.json').write_text(" + repr(json.dumps(receipt)) + ")"
    value["outputs"].append({"path": "app.inputs.json", "format": "link-input-files"})
    if damage == "missing_log":
        value["outputs"] = [row for row in value["outputs"] if row["format"] != "msvc-link-log"]
    write_config(project, **value)
    assert main(args) == (0 if damage is None else 3)
    output = capsys.readouterr()
    if damage in ("log_digest", "missing_log"):
        assert output.err
    else:
        report = json.loads(output.out)
        assert report["archive_file_identity"] == ("pass" if damage is None else "unknown")
        assert report["link_files"]["input_coverage"] == report["storage_verification"] == "unknown"


def test_missing_target_never_comes_from_the_operand_under_test():
    mapping = MAP.replace(" 0001:00000020 _g 00401020 f application.obj\n", "")
    report = verify(*inputs(), mapping)
    assert report["result"] == "unknown"
    assert report["functions"][0]["relocations"][0]["symbol"] == "_g"


@pytest.mark.parametrize("damage", ["missing_function", "non_function", "unplaced", "no_code"])
def test_linked_entry_requires_an_actual_placed_code_function(damage):
    manifest, raw, image = inputs()
    mapping = MAP
    if damage == "missing_function":
        mapping = mapping.replace(" 0001:00000000 _f 00401000 f RUNTIME:runtime.obj\n", "")
    elif damage == "non_function":
        mapping = mapping.replace(" f RUNTIME:", " RUNTIME:")
    elif damage == "unplaced":
        mapping = mapping.replace("0001:00000000 _f 00401000", "0000:00000000 _f 00000000")
    else:
        changed = bytearray(image)
        struct.pack_into("<I", changed, 0x178 + 36, 0x40000040)
        image = bytes(changed)
    assert verify(manifest, raw, image, mapping)["result"] == "unknown"


def test_symbol_map_address_cannot_substitute_for_an_import_identity():
    image = bytearray(image_imports())
    struct.pack_into("<I", image, 0x98 + 16, 0x1000)
    struct.pack_into("<I", image, 0x178 + 36, 0x60000020)
    manifest, raw, image = inputs(coff_fixture(b"\xff\x15\0\0\0\0\xc3", relocation=(2, 1, 6)), bytes(image))
    manifest = replace(manifest, bindings=(replace(manifest.bindings[0], spans=(Span(0x401000, 0x401007),)),))
    mapping = MAP.replace("0001:00000020 _g 00401020 f application.obj", "0001:00000160 _g 00401160 DEMO:DEMO.dll")
    assert verify(manifest, raw, image, mapping)["result"] == "unknown"
    imports = parse_imports(DECLARATION, "code/imports.h")
    assert verify(manifest, raw, image, mapping, import_bindings=imports)["result"] == "pass"
    bad = parse_imports(DECLARATION.replace("DEMO.dll", "WRONG.dll"), "code/imports.h")
    assert verify(manifest, raw, image, mapping, import_bindings=bad)["result"] == "unknown"
    changed = bytearray(image)
    struct.pack_into("<II", changed, 0x98 + 96 + 5 * 8, 0, 0)
    assert verify(manifest, raw, bytes(changed), mapping, import_bindings=imports)["result"] == "fail"


def test_duplicate_member_basename_does_not_select_by_bytes_or_index():
    manifest, raw, image = inputs()
    obj = coff_fixture(b"\xe8\0\0\0\0\xc3", relocation=(1, 1, 20))
    raw = archive([("//", b"one/runtime.obj\0two/runtime.obj\0"), ("/0", obj), ("/16", obj)])
    manifest = replace(manifest, archive_sha256=hashlib.sha256(raw).hexdigest(),
                       bindings=(replace(manifest.bindings[0], unit="one/runtime.obj"),))
    report = verify(manifest, raw, image)
    assert report["result"] == "unknown"
    assert "basename" in report["functions"][0]["reason"]


def configure(project):
    manifest, raw, image = inputs()
    (project / "code/runtime.recovery.h").write_text(render_component(manifest))
    (project / "code/PIN").write_text("sha256 " + hashlib.sha256(image).hexdigest() + "\n")
    (project / "runtime.lib").write_bytes(raw)
    path = project / "recovery/project.json"
    value = json.loads(path.read_text())
    value["source"] = {"roots": ["code"], "seed_roots": [], "image_pin": "code/PIN", "address_space": "pe:va"}
    path.write_text(json.dumps(value))
    script = ("from pathlib import Path; import sys,base64; p=Path(sys.argv[1]); "
              "(p/'app.exe').write_bytes(base64.b64decode(" + repr(base64.b64encode(image).decode()) + ")); "
              "(p/'app.map').write_text(" + repr(MAP) + ")")
    write_config(project, commands=[command(script, "{build}")],
                 outputs=[{"path": "app.exe", "format": "pe32-i386"}, {"path": "app.map", "format": "msvc-map"}])
    return ["components", "check-link", "fixture", "--root", str(project),
            "--archive", "runtime.lib", "--library", "RUNTIME"]


def test_native_cli_builds_and_checks_without_original_or_project_dispatch(project, capsys):
    args = configure(project)
    assert main(args) == 0
    first = json.loads(capsys.readouterr().out)
    assert first["fresh_build"]["result"] == first["result"] == "pass"
    assert first["build_attempts"] == 1
    assert main(args + ["--summary"]) == 0
    second = json.loads(capsys.readouterr().out)
    assert "functions" not in second
    assert first["fresh_build"]["directory"] != second["fresh_build"]["directory"]


@pytest.mark.parametrize("damage", ["missing_map", "invalid_map", "build", "pin"])
def test_fresh_cli_refuses_missing_bad_or_unproduced_evidence(project, capsys, damage):
    args = configure(project)
    if damage == "pin":
        (project / "runtime.lib").write_bytes(b"wrong distribution")
    elif damage == "missing_map":
        write_config(project, outputs=[{"path": "app.exe", "format": "pe32-i386"}])
    else:
        value = json.loads((project / "recovery/build.json").read_text())
        script = value["commands"][0]["argv"][2]
        script += "; (p/'app.map').write_text('broken')" if damage == "invalid_map" else "; sys.exit(1)"
        write_config(project, commands=[command(script, "{build}")])
    assert main(args) == (1 if damage in ("build", "pin") else 3)
    if damage != "missing_map":
        result = json.loads(capsys.readouterr().out)
        assert not result["ready_for_collection"]


@pytest.mark.parametrize("damage", ["source", "import", "archive", "policy", "artifact", "unrelated"])
def test_link_cli_epoch_covers_sources_imports_library_build_and_outputs(project, capsys, monkeypatch, damage):
    args = configure(project)
    original = component_link.verify_linked_toolchain
    def mutate(*values, **kwargs):
        report = original(*values, **kwargs)
        assert report["result"] == "pass"
        path = {"source": project / "code/values.h", "import": project / "code/system.h",
                "archive": project / "runtime.lib", "policy": project / "recovery/build.json",
                "artifact": next((project / "build/checks").glob("build-*/app.map")),
                "unrelated": project / "unrelated.txt"}[damage]
        path.write_text(DECLARATION if damage == "import" else "changed")
        return report
    monkeypatch.setattr(component_link, "verify_linked_toolchain", mutate)
    assert main(args) == 3
    assert capsys.readouterr().err


@pytest.mark.parametrize("damage", ["file_alias", "directory_alias", "changed_before"])
def test_link_artifact_substitutions_are_refused_even_when_bytes_agree(project, capsys, monkeypatch, damage):
    args = configure(project)
    if damage == "changed_before":
        from match_recovery import build_check
        original = build_check.generate_build
        def mutate(*values, **kwargs):
            report = original(*values, **kwargs)
            path = project / report["directory"] / "app.map"
            path.write_text(path.read_text() + "Changed after output capture\n")
            return report
        monkeypatch.setattr(build_check, "generate_build", mutate)
    else:
        original = component_link.verify_linked_toolchain
        def mutate(*values, **kwargs):
            report = original(*values, **kwargs)
            path = next((project / "build/checks").glob("build-*/app.map"))
            if damage == "directory_alias":
                path = path.parent
            moved = path.with_name(path.name + "-moved")
            path.rename(moved)
            path.symlink_to(moved, target_is_directory=damage == "directory_alias")
            return report
        monkeypatch.setattr(component_link, "verify_linked_toolchain", mutate)
    assert main(args) == 3
    assert capsys.readouterr().err
