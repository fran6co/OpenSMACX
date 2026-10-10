"""Identity and generated-table controls for MainInterface's Buffer view."""
import hashlib
import subprocess
from pathlib import Path

from match_recovery.generated_source import parse_generated
from match_recovery.source import read, read_text


ROOT = Path(__file__).resolve().parents[2]
BASE_REVISION = "2c70ab0984af4820a43c46eaf1403e6df30f9fa3"
IMAGE_SHA256 = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"
ORIGINAL_ADDRESS = 0x0045F1F0
ENVELOPE_SHA256 = "fb9dd420c6a6b54025a16e6c472f99adc48f677618b75d4f691d255b030763f1"
ENVELOPE_PREFIX = "/*\nPurpose: Step the receiver back to the subobject"


def envelope(source: str) -> str:
    start = source.index(ENVELOPE_PREFIX)
    end = source.index("*/", start) + 2
    return source[start:end]


def original_record(source: str, path: str):
    records = [row for row in read_text(source, path) if row.address == ORIGINAL_ADDRESS]
    assert len(records) == 1
    return records[0]


def baseline_file(path: str) -> str:
    return subprocess.run(
        ["git", "-C", str(ROOT), "show", f"{BASE_REVISION}:{path}"],
        check=True,
        capture_output=True,
        text=True,
        timeout=30,
    ).stdout


def test_original_adjustor_envelope_moves_verbatim_from_task_baseline():
    baseline = baseline_file("src/mapwin.cpp")
    original = envelope(baseline)
    assert len(original.encode("utf-8")) == 756
    assert hashlib.sha256(original.encode("utf-8")).hexdigest() == ENVELOPE_SHA256

    mapwin = (ROOT / "src/mapwin.cpp").read_text()
    maininterface = (ROOT / "src/maininterface.cpp").read_text()
    assert mapwin == baseline.replace(original + "\n\n", "", 1)
    assert maininterface.replace(original + "\n\n", "", 1) == baseline_file("src/maininterface.cpp")
    assert "ORIGINAL: 0x0045F1F0" not in mapwin
    assert maininterface.count("ORIGINAL: 0x0045F1F0") == 1
    assert envelope(maininterface) == original
    owners = [row for row in read(ROOT / "src") if row.address == ORIGINAL_ADDRESS]
    assert len(owners) == 1
    assert owners[0].path == (ROOT / "src/maininterface.cpp").resolve()

    old = original_record(baseline, "src/mapwin.cpp")
    new = original_record(maininterface, "src/maininterface.cpp")
    for field in (
        "address", "mode", "state", "name", "image_spans", "symbol", "kind",
        "exclusion", "region", "body", "byte_exact", "semantic", "recipe",
        "levers", "ruled_out", "unrecoverable", "deferred", "source_role",
        "source_role_reason", "function_fragment",
    ):
        assert getattr(new, field) == getattr(old, field), field
    assert old.address == ORIGINAL_ADDRESS
    assert old.name == "??_GMainInterface@@WEEE@AEPAXI@Z"
    assert old.symbol == "??_EMainInterface@@WEEE@AEPAXI@Z"
    assert old.image_spans == ((0x0045F1F0, 0x0045F1FB),)
    assert old.byte_exact and not old.semantic
    assert old.body == ""


def test_buffer_view_table_binding_is_start_only_and_genuinely_owned():
    header = (ROOT / "src/maininterface.h").read_text()
    maininterface = (ROOT / "src/maininterface.cpp").read_text()
    graphicwin = (ROOT / "src/graphicwin.h").read_text()
    baseline_header = baseline_file("src/maininterface.h")
    assert "class MainInterface : public GraphicWin, public SubInterface" in header
    assert "class GraphicWin : public Win, public Buffer" in graphicwin
    assert "MainInterface::~MainInterface()" in maininterface
    assert "// ORIGINAL: 0x0045F1C0 ??_GMainInterface@@UAEPAXI@Z" in maininterface
    assert "// RECOVERY_GENERATED: v1 vtable 0x0066C544 ? ??_7MainInterface@@6B@ 0x0045EF10" in header
    assert (
        "// RECOVERY_GENERATED: v1 vtable 0x0066C590 ? "
        "??_7MainInterface@@6BBuffer@@@ 0x0045EF10"
    ) in header
    assert "Buffer-view table start and slot 0 are measured" in header
    assert "full extent" in header and "linked weak fallback selection" in header

    bindings = parse_generated(
        header,
        "src/maininterface.h",
        IMAGE_SHA256,
        address_space="pe:va",
    )
    selected = [binding for binding in bindings if binding.origin.address == 0x0066C590]
    assert len(selected) == 1
    binding = selected[0]
    assert binding.span is None
    assert binding.symbol == "??_7MainInterface@@6BBuffer@@@"
    assert binding.evidence.address == 0x0045EF10

    # This identity is compiler-generated; do not add a copied/manual thunk.
    assert maininterface.count("ORIGINAL: 0x0045F1F0") == 1
    added_header_facts = (
        "// The Buffer-view table start and slot 0 are measured; its full extent,\n"
        "// remaining contents and linked weak fallback selection are not established.\n"
        "// RECOVERY_GENERATED: v1 vtable 0x0066C590 ? ??_7MainInterface@@6BBuffer@@@ 0x0045EF10\n"
    )
    assert header.replace(added_header_facts, "", 1) == baseline_header
