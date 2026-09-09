"""Build-host consistency with alternative seeds and adversarial databases."""

import json
import os
from pathlib import Path
import subprocess
import sys

import pytest

from match_recovery.cli import main
import match_recovery.mapping_check as mappings


MARKER = "// ORIGINAL: 0x00401000 f 0x00401000-0x00401008 BYTE_EXACT\nint f() {return 1;}\n"


@pytest.fixture
def project(tmp_path):
    for name in ("engine/current", "engine/seeds", "build", "recovery", "original"):
        (tmp_path / name).mkdir(parents=True)
    (tmp_path / "original/PIN").write_text("sha256 " + "a" * 64 + "\n")
    (tmp_path / "engine/current/widget.cc").write_text(MARKER)
    (tmp_path / "engine/seeds/widget.cc").write_text(MARKER.replace("0x00401", "0x00402"))
    (tmp_path / "engine/deferred.cc").write_text("#define FORWARD(address, signature) fake(address)\n")
    (tmp_path / "build/compilation.json").write_text(json.dumps([
        {"file": name, "directory": "engine", "arguments": ["compiler-not-run", "-c", name]}
        for name in ("current/widget.cc", "deferred.cc")]))
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1,
        "source": {"image_pin": "original/PIN", "address_space": "custom:offset",
                   "roots": ["engine"], "extensions": [".cc", ".hpp"],
                   "seed_roots": ["engine/seeds"], "include_paths": ["engine"]},
        "mappings": {"compile_commands": "build/compilation.json", "forwarder_macros": ["FORWARD"]},
    }))
    return tmp_path


def test_native_check_is_readonly_and_has_no_compiler_dependency(project):
    before = {path: (path.read_bytes(), path.stat().st_mtime_ns) for path in project.rglob("*") if path.is_file()}
    report = mappings.check_mappings(project)
    assert report["result"] == "pass" and report["entities"] == report["annotations"] == 2
    assert report["build_units"] == 2
    assert not report["persisted"] and not report["ready_for_collection"]
    assert before == {path: (path.read_bytes(), path.stat().st_mtime_ns) for path in project.rglob("*") if path.is_file()}


def test_same_basename_does_not_hide_two_source_candidates(project):
    (project / "engine/seeds/widget.cc").write_text(MARKER)
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    row, = report["findings"]
    assert row["rule"] == "ambiguous_source"
    assert {item["path"] for item in row["candidates"]} == {"engine/current/widget.cc", "engine/seeds/widget.cc"}


def test_complementary_seed_candidates_are_preserved_not_promoted(project):
    (project / "engine/seeds/other").mkdir()
    seed = (project / "engine/seeds/widget.cc").read_text()
    other = project / "engine/seeds/other/widget.cc"
    other.write_text(seed.replace(" BYTE_EXACT", ""))
    report = mappings.check_mappings(project)
    assert report["result"] == "pass"
    assert len(report["seed_alternatives"]) == 1
    assert len(report["seed_alternatives"][0]["candidates"]) == 2
    assert report["annotations"] == 3 and report["entities"] == 2
    assert other.read_text() == seed.replace(" BYTE_EXACT", "")


@pytest.mark.parametrize("placeholder", [False, True])
def test_explicit_product_binding_retains_compatible_unclaimed_seeds(project, placeholder):
    from match_recovery.binding_source import render
    product = project / "engine/current/widget.cc"
    seed = project / "engine/seeds/widget.cc"
    seed.write_text(MARKER.replace(" BYTE_EXACT", ""))
    code = "int f() {\n// BODY GOES HERE.\n}\n" if placeholder else "int f() {return 1;}\n"
    product.write_text(MARKER.splitlines()[0].replace(" BYTE_EXACT", "") + "\n" +
                       render("Own the source definition; retain the seed as a hint.",
                              ["source:engine/seeds/widget.cc"]) + "\n" + code)
    before = seed.read_bytes(), product.read_bytes()
    report = mappings.check_mappings(project)
    assert report["result"] == "pass" and not report["ready_for_collection"]
    group, = report["seed_alternatives"]
    assert group["product_owner"]["path"] == "engine/current/widget.cc"
    assert len(group["candidates"]) == 2
    assert not any(row["legacy_byte_exact"] or row["legacy_semantic"] for row in group["candidates"])
    assert before == (seed.read_bytes(), product.read_bytes())


@pytest.mark.parametrize("damage", ["span", "byte_claim", "semantic_claim", "product", "repeated_seed"])
def test_explicit_binding_does_not_hide_conflicting_or_claimed_candidates(project, damage):
    from match_recovery.binding_source import render
    product = project / "engine/current/widget.cc"
    seed = project / "engine/seeds/widget.cc"
    marker, body = MARKER.replace(" BYTE_EXACT", "").split("\n", 1)
    product.write_text(marker + "\n" + render("Source ownership interpretation.", ["seed evidence"]) + "\n" + body)
    seed.write_text(marker + "\n" + body)
    if damage == "span":
        seed.write_text(seed.read_text().replace("-0x00401008", "-0x00401007"))
    elif damage in {"byte_claim", "semantic_claim"}:
        token = "BYTE_EXACT" if damage == "byte_claim" else "SEMANTIC"
        seed.write_text(marker + " " + token + "\n" + body)
    elif damage == "product":
        (project / "engine/current/other.cc").write_text(product.read_text())
    else:
        seed.write_text(seed.read_text() * 2)
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    assert any(row["rule"] == "ambiguous_source" for row in report["findings"])


@pytest.mark.parametrize("product_claim", ["", "SEMANTIC", "BYTE_EXACT", "BYTE_EXACT SEMANTIC"])
@pytest.mark.parametrize("seed_claim", ["", "SEMANTIC", "BYTE_EXACT", "BYTE_EXACT SEMANTIC"])
def test_explicit_owner_must_carry_all_retained_seed_obligations(project, product_claim, seed_claim):
    from match_recovery.binding_source import render
    product = project / "engine/current/widget.cc"
    seed = project / "engine/seeds/widget.cc"
    marker, body = MARKER.replace(" BYTE_EXACT", "").split("\n", 1)
    product.write_text(marker + " " + product_claim + "\n" +
                       render("Explicit product ownership; retain historical seed.", ["seed evidence"]) +
                       "\n" + body)
    seed.write_text(marker + " " + seed_claim + "\n" + body)
    before = product.read_bytes(), seed.read_bytes()
    report = mappings.check_mappings(project)
    carried = (("BYTE_EXACT" not in seed_claim or "BYTE_EXACT" in product_claim)
               and ("SEMANTIC" not in seed_claim or bool(product_claim)))
    assert report["result"] == ("pass" if carried else "fail")
    assert not report["ready_for_collection"]
    if carried:
        group, = report["seed_alternatives"]
        assert group["product_owner"]["path"] == "engine/current/widget.cc"
        assert len(group["candidates"]) == 2
        assert group["product_owner"]["legacy_byte_exact"] == ("BYTE_EXACT" in product_claim)
    else:
        assert any(row["rule"] == "ambiguous_source" for row in report["findings"])
    assert before == (product.read_bytes(), seed.read_bytes())


@pytest.mark.parametrize("name", ["current", "seeds"])
def test_repeated_marker_in_one_file_is_ambiguous_even_for_seeds(project, name):
    path = project / f"engine/{name}/widget.cc"
    path.write_text(path.read_text() * 2)
    assert mappings.check_mappings(project)["findings"][0]["rule"] == "ambiguous_source"


def test_nested_unbuilt_host_and_seed_in_build_are_not_exempted(project):
    path = project / "engine/current/nested"
    path.mkdir()
    (path / "widget.cc").write_text(MARKER.replace("0x00401", "0x00403"))
    report = mappings.check_mappings(project)
    assert report["findings"] == [{"rule": "unbuilt_host", "path": "engine/current/nested/widget.cc",
                                   "entities": report["findings"][0]["entities"]}]
    database = project / "build/compilation.json"
    entries = json.loads(database.read_text())
    entries.append({"directory": "engine", "file": "seeds/widget.cc", "command": "compiler-not-run"})
    database.write_text(json.dumps(entries))
    assert {row["rule"] for row in mappings.check_mappings(project)["findings"]} == {"unbuilt_host", "seed_build_input"}


@pytest.mark.parametrize("address,category", [("0x00401000", "compiled_claim"), ("0x00402000", "seed_claim")])
def test_forwarder_and_claim_conflict_regardless_of_claim_home(project, address, category):
    (project / "engine/deferred.cc").write_text(f"int f() {{return FORWARD({address}, signature)();}}\n")
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    row, = report["findings"]
    assert row["rule"] == "claimed_forwarder" and row["classification"] == category


def test_nonliteral_forwarder_is_unknown_not_silently_ignored(project):
    (project / "engine/deferred.cc").write_text("FORWARD(BASE + 8, signature)();\n")
    report = mappings.check_mappings(project)
    assert report["result"] == "unknown" and report["errors"]


def test_unclaimed_product_does_not_borrow_a_seeds_claim_classification(project):
    (project / "engine/current/widget.cc").write_text(MARKER.replace(" BYTE_EXACT", ""))
    (project / "engine/seeds/widget.cc").write_text(MARKER)
    (project / "engine/deferred.cc").write_text("FORWARD(0x00401000, signature)();\n")
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    row, = (row for row in report["findings"] if row["rule"] == "claimed_forwarder")
    assert row["classification"] == "seed_claim"


def test_unbuilt_product_claim_is_not_mislabeled_as_seed_evidence(project):
    database = project / "build/compilation.json"
    database.write_text(json.dumps(json.loads(database.read_text())[1:]))
    (project / "engine/deferred.cc").write_text("FORWARD(0x00401000, signature)();\n")
    report = mappings.check_mappings(project)
    assert report["result"] == "fail"
    row, = (row for row in report["findings"] if row["rule"] == "claimed_forwarder")
    assert row["classification"] == "unbuilt_product_claim"


def test_valid_arguments_do_not_hide_a_malformed_command_field(project):
    database = project / "build/compilation.json"
    rows = json.loads(database.read_text())
    rows[0]["command"] = 123
    database.write_text(json.dumps(rows))
    with pytest.raises(ValueError, match="command"):
        mappings.check_mappings(project)


def test_forwarder_lexer_respects_comments_strings_splices_and_macro_definitions():
    text = '''#define FORWARD(address, signature) something(address)
// FORWARD(0x11, signature)();
/* FORWARD(0x12, signature)(); */
const char *a = "FORWARD(0x13, signature)";
const char *b = R"tag("FORWARD(0x14, signature)")tag";
FOR/**/WARD(0x15, signature)();
FORWARD(0x16u, signature)();
FOR\\
WARD(23, signature)();
// Continued comment \\
FORWARD(0x18, signature)();
'''
    sites, errors = mappings.forwarder_sites(text, ("FORWARD",))
    assert errors == []
    assert [(row["address"], row["line"]) for row in sites] == [(22, 7), (23, 8)]


@pytest.mark.parametrize("data", [b"[]", b"{}", b'[{}]', b'[{"file":"a","file":"b"}]',
                                  b'[{"file":NaN}]'])
def test_invalid_compilation_database_cannot_prove_empty_success(project, data):
    (project / "build/compilation.json").write_bytes(data)
    with pytest.raises(ValueError):
        mappings.check_mappings(project)


@pytest.mark.parametrize("change", ["source", "database", "policy", "added"])
def test_changed_inputs_during_check_are_refused(project, monkeypatch, change):
    actual = mappings.forwarder_sites
    def alter(*args):
        result = actual(*args)
        path = project / {"source": "engine/current/widget.cc", "database": "build/compilation.json",
                          "policy": "recovery/project.json", "added": "engine/new.hpp"}[change]
        path.write_bytes((path.read_bytes() if path.exists() else b"") + b"\n")
        return result
    monkeypatch.setattr(mappings, "forwarder_sites", alter)
    with pytest.raises(ValueError):
        mappings.check_mappings(project)


def test_source_directory_alias_cannot_hide_mappings(project):
    (project / "engine/alias").symlink_to(project / "engine/seeds", target_is_directory=True)
    with pytest.raises(ValueError, match="symlinks"):
        mappings.check_mappings(project)


def test_installed_command_boundary_without_optional_dependencies(project):
    import match_recovery
    environment = dict(os.environ, PYTHONPATH=str(Path(match_recovery.__file__).parents[1]))
    result = subprocess.run([sys.executable, "-S", "-m", "match_recovery.cli", "check-mappings",
                             "--root", str(project), "--summary"],
                            cwd=project, env=environment, capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    assert json.loads(result.stdout)["annotations"] == 2


def test_cli_summary_and_unknown_status(project, capsys):
    assert main(["check-mappings", "--root", str(project), "--summary"]) == 0
    report = json.loads(capsys.readouterr().out)
    assert report["seed_alternative_groups"] == report["forwarder_sites"] == 0
    (project / "engine/deferred.cc").write_text("FORWARD(UNKNOWN, fn)();\n")
    assert main(["check-mappings", "--root", str(project)]) == 3
    assert json.loads(capsys.readouterr().out)["errors"]
