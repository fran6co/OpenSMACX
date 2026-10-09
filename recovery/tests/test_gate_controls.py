"""Opt-in real-project damage controls for the installed native gate.

Run with OPENSMACX_LIVE_GATE_CONTROLS=1 and an isolated VC6_PREFIX. Each case
invokes the FULL gate in a disposable detached checkout, never the working source
tree. Use pytest -k to select cases; fresh before/after baselines still run.
These are project tests (their damage anchors are project-specific), not another
project CLI or framework adapter. Reports live only in ignored build storage.
"""

from contextlib import contextmanager
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

import pytest

from match_recovery.processes import run_bounded
from match_recovery.source_config import SourceConfiguration
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


class Edits:
    """Undo only the exact damage this control wrote; preserve other editors."""

    def __init__(self, root):
        self.root = root.resolve()
        self.original = {}
        self.expected = {}

    def path(self, name):
        path = self.root / name
        assert (path.is_relative_to(self.root) and path.resolve() == path
                and path.is_file() and path.stat().st_nlink == 1), "unsafe control source path: " + name
        return path

    def read(self, name):
        return self.path(name).read_bytes().decode("utf-8")

    def write(self, name, value):
        path = self.path(name)
        current = path.read_bytes()
        if name not in self.original:
            self.original[name] = current
            self.expected[name] = current
        assert current == self.expected[name], f"concurrent edit before damage: {name}"
        encoded = value.encode("utf-8")
        path.write_bytes(encoded)
        self.expected[name] = encoded

    def replace(self, name, old, new):
        text = self.read(name)
        assert text.count(old) == 1, f"damage anchor is missing or ambiguous: {name}: {old}"
        self.write(name, text.replace(old, new, 1))

    def append(self, name, text):
        self.write(name, self.read(name) + text)

    def claim(self, name="src/jackal/palette.cpp"):
        # Derive the live marker; do not pin a former seed or a changing count.
        return next(line for line in self.read(name).splitlines()
                    if re.fullmatch(r"// ORIGINAL: 0x\w+ \S+ \S+ BYTE_EXACT", line))

    def restore(self):
        conflicts = []
        for name, original in self.original.items():
            try:
                path = self.path(name)
            except AssertionError:
                conflicts.append(name)
                continue
            if path.read_bytes() == self.expected[name]:
                path.write_bytes(original)
            else:
                conflicts.append(name)
        assert not conflicts, "preserved concurrent changes in control checkout: " + ", ".join(conflicts)


@contextmanager
def damage(root):
    edits = Edits(root)
    try:
        yield edits
    finally:
        edits.restore()


def slack(edits):
    name = "recovery/audits.json"
    config = json.loads(edits.read(name))
    audit = config["audits"]["compiler-work"]
    counter = next(row["name"] for row in audit["counters"]
                   if row["name"] == "field_accessors.cpp markers")
    audit["ceilings"][counter] += 5
    edits.write(name, json.dumps(config, indent=2) + "\n")


def wrong_image_pin(edits):
    configuration = SourceConfiguration("src/IMAGE_PIN", "pe:va")
    original = image_pin(edits.root, configuration)
    changed = ("1" if original[0] == "0" else "0") + original[1:]
    edits.replace(configuration.image_pin, "sha256 " + original, "sha256 " + changed)
    # A textual edit alone is insufficient: a comment can mention sha256 too.
    assert image_pin(edits.root, configuration) == changed


def swap_claim(edits):
    old = edits.claim()
    edits.replace("src/jackal/palette.cpp", old, old.removesuffix(" BYTE_EXACT"))
    candidate = next(line for line in edits.read("src/caviar.cpp").splitlines()
                     if line.startswith("// ORIGINAL:") and "BYTE_EXACT" not in line and "SEMANTIC" not in line)
    edits.replace("src/caviar.cpp", candidate, candidate + " BYTE_EXACT")


def conflicting_map(edits):
    # Conflict across compiled owners, rather than a retired seed directory.
    edits.append("src/caviar.cpp", "\n" + edits.claim() + "\n")


def symbol(edits, name):
    marker = edits.claim()
    edits.replace("src/jackal/palette.cpp", marker + "\n", marker + "\n// symbol    " + name + "\n")


@dataclass(frozen=True)
class Case:
    name: str
    stage: str | None
    result: str
    mutate: object
    subject: tuple[str, str, str] | None = None
    layout_subject: tuple[str, str] | None = None


CASES = (
    Case("compiler_work_slack", "audit:compiler-work", "fail", slack),
    Case("compiler_work_growth", "audit:compiler-work", "fail",
         lambda e: e.append("src/jackal/palette.h", "\nstatic int *const g_00dead01 = (int *)0x00DEAD01;\n")),
    Case("compiler_work_ignores_block_comments", "audit:compiler-work", "pass",
         lambda e: e.append("src/jackal/palette.h", "\n/*\n g_00dead02 = (int *)0x00DEAD02;\n*/\n")),
    Case("bounded_pointer_initial_value", "initial-data", "fail",
         lambda e: e.replace("src/jackal/radiobutton.cpp", "Sprite *RadioButtonDefault1 = &g_RADIOBUTTON_SPRITE_1;",
                             "Sprite *RadioButtonDefault1 = 0;")),
    Case("bounded_pointer_target", "initial-data", "fail",
         lambda e: e.replace("src/jackal/radiobutton.cpp", "Sprite *RadioButtonDefault1 = &g_RADIOBUTTON_SPRITE_1;",
                             "Sprite *RadioButtonDefault1 = &g_RADIOBUTTON_SPRITE_2;")),
    Case("dropped_initial_value", "project:trailing-address-initial-values", "fail",
         lambda e: e.replace("src/sprite.cpp", "SpriteScaleNumerator = 1;", "SpriteScaleNumerator;")),
    Case("image_pin", None, "unknown", wrong_image_pin),
    Case("source_symbol_missing", "symbols", "unknown",
         lambda e: symbol(e, "?totally_bogus_harness@@YAXXZ"),
         ("functions", "symbol", "?totally_bogus_harness@@YAXXZ")),
    Case("source_symbol_data_binding", "symbols", "fail",
         lambda e: symbol(e, "?PaletteActive@@3PAVPalette@@A"),
         ("functions", "symbol", "?PaletteActive@@3PAVPalette@@A")),
    Case("live_references", "live-references", "fail",
         lambda e: e.append("README.md", "\nRun `uv run tools/harness_not_a_tool_9f3a.py` now.\n")),
    Case("source_mapping_conflict", "mappings", "fail", conflicting_map),
    Case("class_model_growth", "audit:class-debt", "fail",
         lambda e: e.append("src/jackal/palette.h", "\n  void UNK99(int);\n")),
    Case("storage_identity_conflict", "audit:storage", "fail",
         lambda e: e.append("src/jackal/palette.h", "\nextern int StorageConflictControl; // 0x009BB438\n")),
    Case("scaled_pointer_offset", "audit:storage", "fail",
         lambda e: e.append("src/jackal/palette.h", "\ninline int *storage_scale_control(Palette *parent) {\n"
                            "  return reinterpret_cast<int *>(parent + 0xcc);\n}\n")),
    Case("historical_claim_removed", "claim-identities", "fail",
         lambda e: e.replace("src/jackal/palette.cpp", e.claim(), e.claim().replace("// ORIGINAL:", "// ORIGINAl:"))),
    Case("same_count_claim_swap", "claim-identities", "fail", swap_claim),
    Case("conventional_offset_disagrees", "layout", "fail",
         lambda e: e.replace("src/basewin.h", "uint32_t field_1D04C_;  // 0x1D04C",
                            "uint32_t field_1D04C_;  // 0x1D0FC"),
         layout_subject=("BaseWin", "field_1D04C_")),
    Case("unobserved_offset_disagrees", "layout", "fail",
         lambda e: e.replace("src/spritebox.h",
                             "RECOVERY_MEMBER: v1 SpriteList field_24_ 0x24 0x005EB29F",
                             "RECOVERY_MEMBER: v1 SpriteList field_24_ 0x28 0x005EB29F"),
         layout_subject=("SpriteList", "field_24_")),
    Case("source_layout_expectation", "layout", "fail",
         lambda e: e.replace("src/jackal/radiobutton.h",
                            "RECOVERY_MEMBER: v3 RadioButton unchecked_sprite_ Sprite* 0x4 0x0060D750",
                            "RECOVERY_MEMBER: v3 RadioButton unchecked_sprite_ Sprite* 0x8 0x0060D750")),
    Case("annotation_identity", "source", "fail",
         lambda e: e.replace("packages/match-recovery/src/match_recovery/source/writer.py",
                            "    if record.origin is None:\n        return False",
                            "    if True:\n        return False")),
    Case("annotation_marker_spelling", "source", "fail",
         lambda e: e.replace("src/jackal/palette.cpp", e.claim(), e.claim().replace("// ORIGINAL: ", "// ORIGINAL:  ", 1))),
    Case("fresh_build_header_damage", "build", "fail",
         lambda e: e.append("src/jackal/palette.h", "\n#error native_gate_header_damage_control\n")),
)


def assert_control(case, code, report, stderr):
    if case.stage is None:
        assert code == 3 and report is None and "configured pinned original image" in stderr
        return
    assert report is not None and report["check"] == "project_gate"
    assert code == report["exit_code"] and not report["ready_for_collection"]
    # Never accept an abort, another red check, or missing coverage in place of
    # the intended control. Baseline unknowns cannot satisfy a missing-name test.
    stages = {row["name"]: row for row in report["checks"]}
    assert "freshness" not in stages
    assert stages[case.stage]["result"] == case.result
    if case.result == "pass":
        assert code in (0, 3)
    elif case.result == "fail":
        assert code == 1
    else:
        assert code in (1, 3)
    if case.subject:
        collection, field, expected = case.subject
        rows = [row for row in stages[case.stage]["report"][collection] if row[field] == expected]
        assert rows and all(row["result"] == case.result for row in rows)
    if case.layout_subject:
        name, member = case.layout_subject
        rows = [row for row in stages["layout"]["report"]["expectations"]
                if row["expectation"]["class_name"] == name and row["expectation"].get("member") == member]
        assert any(row["result"] == case.result for row in rows), "intended class/member counterexample was not observed"
    if case.name == "source_symbol_missing":
        # The same damage covers both former missing-name cases in one full
        # compiler run: unknown emission must also fail the built byte claim.
        assert stages["matches"]["result"] == "fail"
        rows = [row for row in stages["matches"]["report"]["regressed"] if row["symbol"] == case.subject[2]]
        assert rows and all(row["result"] == "fail" and row["verdict"] == "UNRESOLVED" for row in rows)


def git(root, *arguments):
    return subprocess.run(["git", *arguments], cwd=root, check=True, capture_output=True, text=True).stdout


@pytest.fixture(scope="module")
def live_checkout():
    # All actual damage is isolated. The test checkout uses its own uv package
    # installation too, so code-side writer damage reaches that gate subprocess.
    assert not git(ROOT, "status", "--porcelain").strip(), "checkpoint changes before live controls"
    assert os.environ.get("VC6_PREFIX"), "live controls need an explicitly isolated VC6_PREFIX"
    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe")).resolve()
    assert image.is_file(), "live controls require the pinned original executable"
    jobs = int(os.environ.get("OPENSMACX_GATE_JOBS", "1"))
    assert jobs > 0
    artifacts = ROOT / "build/recovery/gate-controls"
    artifacts.mkdir(parents=True, exist_ok=True)
    parent = Path(tempfile.mkdtemp(prefix="run-", dir=artifacts))
    checkout = parent / "checkout"
    git(ROOT, "worktree", "add", "--detach", str(checkout), "HEAD")
    environment = pytest.MonkeyPatch()
    environment.setenv("UV_PROJECT", str(checkout))
    environment.setenv("UV_PROJECT_ENVIRONMENT", str(checkout / ".venv"))
    environment.delenv("PYTHONPATH", raising=False)
    environment.delenv("VIRTUAL_ENV", raising=False)

    def run(name):
        print(f"native gate control {name}: starting in {checkout}", flush=True)
        outcome = run_bounded(["uv", "run", "recover", "check", "--image", str(checkout / ".opensmacx/game/terranx_original.exe"),
                               "--jobs", str(jobs), "--summary"], cwd=checkout, timeout=5400)
        # Logs are disposable evidence outside the child checkout. A failed
        # assertion or cleanup must not erase the report needed to investigate.
        (parent / (name + ".stdout.json")).write_bytes(outcome.stdout)
        (parent / (name + ".stderr.log")).write_bytes(outcome.stderr)
        (parent / (name + ".exit.json")).write_text(json.dumps({"exit_code": outcome.returncode}))
        output = json.loads(outcome.stdout) if outcome.stdout.strip() else None
        print(f"native gate control {name}: exit {outcome.returncode}; report {parent / (name + '.stdout.json')}", flush=True)
        return outcome.returncode, output, outcome.stderr.decode(errors="replace")

    def baseline(name):
        code, report, stderr = run(name)
        assert code in (0, 3) and report is not None, stderr
        assert report["check"] == "project_gate" and code == report["exit_code"]
        stages = {row["name"]: row["result"] for row in report["checks"]}
        assert stages["preparation"] == stages["build"] == "pass"
        assert not ({"fail"} & set(stages.values())) and "freshness" not in stages
        assert {"matches", *(case.stage for case in CASES if case.stage is not None)} <= stages.keys()

    try:
        target = checkout / ".opensmacx/game/terranx_original.exe"
        target.parent.mkdir(parents=True)
        shutil.copy2(image, target)
        identity = run_bounded(["uv", "run", "python", "-c", "import match_recovery; print(match_recovery.__file__)"],
                               cwd=checkout, timeout=120)
        assert identity.returncode == 0, identity.stderr.decode(errors="replace")
        imported = Path(identity.stdout.decode().strip()).resolve()
        assert imported.is_relative_to(checkout / "packages/match-recovery"), "control gate imported the wrong package checkout"
        baseline("before")
        try:
            yield checkout, run
        finally:
            baseline("after")
    finally:
        environment.undo()
        # No force: unexpected source edits leave the checkout for inspection.
        git(ROOT, "worktree", "remove", str(checkout))


@pytest.mark.skipif(os.environ.get("OPENSMACX_LIVE_GATE_CONTROLS") != "1", reason="opt-in full real-project gate controls")
@pytest.mark.parametrize("case", CASES, ids=lambda case: case.name)
def test_live_native_gate_control(live_checkout, case):
    checkout, run = live_checkout
    with damage(checkout) as edits:
        case.mutate(edits)
        assert_control(case, *run(case.name))


def test_control_restore_preserves_bytes_and_line_endings(tmp_path):
    path = tmp_path / "source.cpp"
    original = b"int value;\r\n// untouched \xff\r\n"
    path.write_bytes(original)
    edits = Edits(tmp_path)
    # read() deliberately refuses non-UTF8 damage anchors; never replacement-decode
    # and silently rewrite unknown source bytes as the former harness did.
    with pytest.raises(UnicodeDecodeError):
        edits.read("source.cpp")
    assert path.read_bytes() == original
    original = b"int value;\r\n"
    path.write_bytes(original)
    with damage(tmp_path) as edits:
        edits.replace("source.cpp", "value", "changed")
        assert path.read_bytes() == b"int changed;\r\n"
    assert path.read_bytes() == original


def test_control_restore_never_overwrites_another_edit(tmp_path):
    path = tmp_path / "source.cpp"
    path.write_text("original")
    with pytest.raises(AssertionError, match="preserved concurrent"):
        with damage(tmp_path) as edits:
            edits.replace("source.cpp", "original", "controlled damage")
            path.write_text("another editor")
    assert path.read_text() == "another editor"


def test_control_anchor_must_be_unique(tmp_path):
    (tmp_path / "source.cpp").write_text("duplicate duplicate")
    with damage(tmp_path) as edits:
        with pytest.raises(AssertionError, match="ambiguous"):
            edits.replace("source.cpp", "duplicate", "replacement")
    assert (tmp_path / "source.cpp").read_text() == "duplicate duplicate"


@pytest.mark.parametrize("pin", ["0" * 64, "1" * 64])
def test_pin_damage_changes_parsed_identity_not_sha256_comment(tmp_path, pin):
    path = tmp_path / "src/IMAGE_PIN"
    path.parent.mkdir()
    comment = "# Historical command: grep -rn sha256 tools decomp\n"
    original = comment + "sha256 " + pin + "\n"
    path.write_text(original)
    configuration = SourceConfiguration("src/IMAGE_PIN", "pe:va")
    case = next(case for case in CASES if case.name == "image_pin")
    with damage(tmp_path) as edits:
        case.mutate(edits)
        assert image_pin(tmp_path, configuration) != pin
        assert path.read_text().splitlines()[0] == comment.rstrip("\n")
    assert path.read_text() == original
    assert image_pin(tmp_path, configuration) == pin


@pytest.mark.parametrize("kind", ["symlink", "hardlink"])
def test_control_refuses_aliased_source_before_writing(kind, tmp_path):
    (tmp_path / "original.cpp").write_text("original")
    path = tmp_path / "alias.cpp"
    if kind == "symlink":
        path.symlink_to(tmp_path / "original.cpp")
    else:
        os.link(tmp_path / "original.cpp", path)
    with damage(tmp_path) as edits:
        with pytest.raises(AssertionError, match="unsafe"):
            edits.replace("alias.cpp", "original", "damaged")
    assert (tmp_path / "original.cpp").read_text() == "original"


def test_control_rejects_an_unrelated_red_stage_or_baseline_unknown():
    case = next(case for case in CASES if case.name == "source_symbol_missing")
    report = {"check": "project_gate", "exit_code": 1, "ready_for_collection": False, "checks": [
        {"name": "matches", "result": "fail", "report": {"regressed": [
            {"symbol": case.subject[2], "result": "fail", "verdict": "UNRESOLVED"}]}},
        {"name": "symbols", "result": "unknown", "report": {"functions": [
            {"symbol": "?unrelated@@YAXXZ", "result": "unknown"}]}}]}
    with pytest.raises(AssertionError):
        assert_control(case, 1, report, "")
    report["checks"][1]["report"]["functions"][0]["symbol"] = case.subject[2]
    assert_control(case, 1, report, "")
    report["checks"][0]["report"]["regressed"] = []
    with pytest.raises(AssertionError):
        assert_control(case, 1, report, "")


def test_comment_control_requires_its_audit_to_pass():
    case = next(case for case in CASES if case.name == "compiler_work_ignores_block_comments")
    report = {"check": "project_gate", "exit_code": 3, "ready_for_collection": False, "checks": [
        {"name": "audit:compiler-work", "result": "unknown", "report": {}}]}
    with pytest.raises(AssertionError):
        assert_control(case, 3, report, "no old console wording here")


@pytest.mark.parametrize("case", CASES, ids=lambda case: case.name)
def test_damage_anchor_still_applies_and_restores_private_source(case, tmp_path):
    # Exercise every project anchor cheaply, without compiling or touching live
    # source. A legitimate rename/rehoming should update its dependent control
    # before an hours-long full-gate run discovers the stale anchor.
    names = ["src/jackal/palette.cpp", "src/jackal/palette.h", "src/caviar.cpp", "src/jackal/radiobutton.cpp",
             "src/jackal/radiobutton.h", "src/basewin.h", "src/spritebox.h", "src/sprite.cpp", "src/IMAGE_PIN", "README.md",
             "recovery/audits.json", "packages/match-recovery/src/match_recovery/source/writer.py"]
    original = {}
    for name in names:
        path = tmp_path / name
        path.parent.mkdir(parents=True, exist_ok=True)
        original[name] = (ROOT / name).read_bytes()
        path.write_bytes(original[name])
    with damage(tmp_path) as edits:
        case.mutate(edits)
        assert edits.original and any((tmp_path / name).read_bytes() != original[name] for name in edits.original)
    assert all((tmp_path / name).read_bytes() == data for name, data in original.items())


def test_renamed_scale_initializer_damage_is_still_observed(tmp_path):
    # Merely finding the renamed variable is insufficient: the conventional
    # initializer lens also requires its trailing address hint. Native bounded
    # object bindings strengthen that evidence but do not replace this stage's
    # independent control until the stage itself is retired.
    from match_recovery.initializer_check import check_initializers

    image = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not image.is_file():
        pytest.skip("pinned original required for initializer damage control")
    for name in ("src/sprite.cpp", "src/IMAGE_PIN", "recovery/project.json"):
        target = tmp_path / name
        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_bytes((ROOT / name).read_bytes())
    before = check_initializers(tmp_path, image)
    owner, = [row for row in before["observations"] if row["symbol"] == "SpriteScaleNumerator"]
    assert owner["image_nonzero"] and not owner["source_zero"]
    case = next(case for case in CASES if case.name == "dropped_initial_value")
    with damage(tmp_path) as edits:
        case.mutate(edits)
        damaged = check_initializers(tmp_path, image)
        assert damaged["result"] == "fail"
        assert any(row["symbol"] == "SpriteScaleNumerator" for row in damaged["findings"])
    restored = check_initializers(tmp_path, image)
    assert restored["findings"] == before["findings"]
    assert restored["observations"] == before["observations"]
