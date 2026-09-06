import base64
import hashlib
import json

import pytest

from match_recovery.adapters.opensmacx import inventory, legacy_snapshot
from match_recovery.cli import main
from match_recovery.store import ObjectStore


PIN = "a" * 64
TEXT = b"""// ORIGINAL: 0x00401000 ?f@@YAXXZ 0x00401000-0x00401008 BYTE_EXACT
// kind      game
// TRIED: this spelling failed before the class changed
// LEVER: order the later member order restored matching
// arbitrary ongoing context, deliberately outside the parsed vocabulary
void f() {}
"""


@pytest.fixture
def project(tmp_path):
    src = tmp_path / "src"
    src.mkdir()
    (src / "IMAGE_PIN").write_text("sha256 " + PIN + "\n")
    (src / "a.cpp").write_bytes(TEXT)
    return tmp_path


def test_import_keeps_exact_source_and_treats_claims_as_historical(project):
    path = project / "src" / "a.cpp"
    path.write_bytes(TEXT + b"// old encoding: \xff\n")
    before = path.read_bytes()
    snapshot = legacy_snapshot(project)
    assert base64.b64decode(snapshot["files"]["src/a.cpp"]["content_base64"]) == before
    assert snapshot["records"][0]["tried"]
    assert snapshot["records"][0]["levers"]
    assert len(snapshot["historical_obligations"]) == 1
    assert inventory(snapshot)["verification"] == "not_measured"
    assert not snapshot["image_verified"]
    assert path.read_bytes() == before


def test_seeds_do_not_override_product_source(project):
    seeds = project / "src" / "unrecovered"
    seeds.mkdir()
    (seeds / "seed.cpp").write_bytes(TEXT)
    snapshot = legacy_snapshot(project)
    summary = inventory(snapshot)
    assert summary["entities"] == 1
    assert summary["annotations"] == 2
    assert summary["candidates"] == {"source": 1, "legacy_seed": 1}
    assert summary["source_conflicts"] == []
    assert len(snapshot["historical_obligations"]) == 2


def test_conflicting_product_copies_are_reported_not_selected(project):
    (project / "src" / "b.cpp").write_bytes(TEXT)
    assert len(inventory(legacy_snapshot(project))["source_conflicts"]) == 1


def test_source_names_and_worktree_paths_are_not_identity(project):
    first = legacy_snapshot(project)["records"][0]["entity_id"]
    original = project / "src" / "a.cpp"
    original.rename(project / "src" / "renamed.cpp")
    assert legacy_snapshot(project)["records"][0]["entity_id"] == first


def test_wrong_image_pin_is_rejected(project, tmp_path):
    image = tmp_path / "fixture.bin"
    image.write_bytes(b"synthetic image, not game bytes")
    with pytest.raises(ValueError, match="image digest mismatch"):
        legacy_snapshot(project, image)
    pin = hashlib.sha256(image.read_bytes()).hexdigest()
    (project / "src" / "IMAGE_PIN").write_text("sha256 " + pin)
    assert legacy_snapshot(project, image)["image_verified"]


def test_empty_import_is_an_error(project):
    (project / "src" / "a.cpp").write_text("// no annotations\n")
    with pytest.raises(ValueError, match="empty migration"):
        legacy_snapshot(project)


def test_context_hint_exposes_real_constructor_hidden_by_wrapper_binding(project):
    path = project / "src/a.cpp"
    path.write_text("// ORIGINAL: 0x00401000 ??0Widget@@QAE@XZ 0x00401000-0x00401008\n"
                    "// symbol    ?construct@Widget@@QAEPAV1@XZ\n"
                    "Widget *Widget::construct() { return this; }\n"
                    "Widget::Widget() {}\n")
    snapshot = legacy_snapshot(project)
    hint = snapshot["records"][0]["binding_hint"]
    assert hint["candidate_definition"] == "Widget::Widget" and hint["candidate_line"] == 4
    assert snapshot["records"][0]["emitted_symbol_hint"] == "?construct@Widget@@QAEPAV1@XZ"
    assert path.read_text().count("// symbol") == 1  # No implicit rebinding or stamping.
    path.write_text(path.read_text().replace("Widget::Widget() {}", "Widget::Widget();"))
    assert legacy_snapshot(project)["records"][0]["binding_hint"] is None


def test_old_semantic_claims_keep_their_own_obligations(project):
    (project / "src" / "a.cpp").write_bytes(TEXT.replace(b"BYTE_EXACT", b"SEMANTIC"))
    snapshot = legacy_snapshot(project)
    assert snapshot["historical_obligations"][0]["dimension"] == "instruction_shape"
    assert inventory(snapshot)["historical_byte_obligations"] == 0
    assert inventory(snapshot)["historical_semantic_claims"] == 1


def test_cli_json_is_clean_and_import_goes_to_ignored_storage(project, capsys):
    assert main(["inventory", "--root", str(project)]) == 0
    output = capsys.readouterr()
    assert json.loads(output.out)["historical_byte_obligations"] == 1
    assert output.err == ""
    assert not (project / ".opensmacx").exists()
    assert main(["import", "legacy", "--root", str(project)]) == 0
    output = json.loads(capsys.readouterr().out)
    store = ObjectStore(project / ".opensmacx" / "recovery" / "objects")
    assert store.get(output["snapshot_id"])["records"][0]["legacy_byte_exact"]
    assert not (project / "recovery").exists()
    assert "event_sequence" not in output


def test_cli_errors_are_not_green_or_json_contamination(tmp_path, capsys):
    assert main(["inventory", "--root", str(tmp_path)]) == 3
    output = capsys.readouterr()
    assert output.out == ""
    assert output.err.startswith("recover:")
