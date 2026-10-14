"""Seven native claim promotions; not linked/reference/game acceptance."""
from dataclasses import fields, replace
from pathlib import Path
import subprocess
import re

import pytest

from match_recovery.source import read_text
from match_recovery.source.writer import write_bytes

ROOT = Path(__file__).resolve().parents[2]
BASE = "0e6b4b490aea7df0e4dc5ce7961eae3de4f129db"
NETFIFO_STRUCTURE_BASE = "7a20029edfe4fb109e048bdcdce23f0e44cc0117"
SELECTED = {0x59E950, 0x5D4BB0, 0x5CBBC0, 0x631B30,
            0x633A50, 0x634330, 0x44C870}


def historical_core_claim_view(text, relative):
    """Test-only projection of the seven flags, never a source-file write.

    Historical guards keep their own original baselines. This bridge reverses
    only this separately verified publication, preserving every other byte.
    The full-file test below requires the live stronger claims, so a removed
    promotion cannot hide behind this historical projection.
    """
    path = ROOT / relative
    selected = [row for row in read_text(text, path) if row.address in SELECTED]
    if not selected:
        return text
    baseline = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    old = {row.address: row for row in read_text(baseline.decode(), path)}
    restored = []
    for row in selected:
        original = old[row.address]
        assert (row.byte_exact, row.semantic) in {
            (original.byte_exact, original.semantic), (True, False)}
        restored.append(replace(row, byte_exact=original.byte_exact,
                                semantic=original.semantic))
    return write_bytes(text.encode(), restored).decode()


def historical_netfifo_structure_view(text, relative, *, header_text=None):
    """Reverse only the approved direct destructor-body recovery in NetFifo.

    The fixed pre-change source/header remain the comparison oracle. This
    bridge requires the destructor's drain sequence to equal the immutable
    close body followed by lock destruction, then projects only that body,
    its exact binding and the import annotation back to the fixed baseline.
    """
    if relative != "src/net_fifo.cpp":
        return text
    from test_netwin_owner_behavior import extract_function

    baseline_source = subprocess.check_output(
        ["git", "show", NETFIFO_STRUCTURE_BASE + ":src/net_fifo.cpp"],
        cwd=ROOT, text=True)
    baseline_header = subprocess.check_output(
        ["git", "show", NETFIFO_STRUCTURE_BASE + ":src/net_fifo.h"],
        cwd=ROOT, text=True)
    if header_text is None:
        header_text = (ROOT / "src/net_fifo.h").read_text()

    source_path = ROOT / relative
    original_rows = read_text(baseline_source, source_path)
    current_rows = read_text(text, source_path)
    original_dtor = [row for row in original_rows if row.address == 0x6339E0]
    current_dtor = [row for row in current_rows if row.address == 0x6339E0]
    assert len(original_dtor) == len(current_dtor) == 1
    original_dtor, = original_dtor
    current_dtor, = current_dtor
    assert current_dtor.byte_exact is True and current_dtor.semantic is False, "native destructor claim was withdrawn or changed"
    text = write_bytes(text.encode(), [replace(
        current_dtor, byte_exact=original_dtor.byte_exact,
        semantic=original_dtor.semantic)]).decode()

    def compact(value):
        return re.sub(r"\s+", "", value)

    baseline_close = extract_function(baseline_source, "void NetFifo::close()")
    baseline_destructor = extract_function(baseline_source, "NetFifo::~NetFifo()")
    candidate_destructor = extract_function(text, "NetFifo::~NetFifo()")
    binding_marker = "// RECOVERY_BINDING: v1 "
    binding_index = text.rfind(binding_marker, 0, text.index("NetFifo::~NetFifo()"))
    assert binding_index >= 0
    binding_end = text.index("\n", binding_index)
    candidate_binding = text[binding_index:binding_end]
    expected_binding = ('// RECOVERY_BINDING: v1 {"evidence": ["pinned span 0x006339E0-0x00633A42", '
                        '"NetFifo is a 0x24-byte embedded member at Net+0xE8/+0x10C/+0x130", '
                        '"original instructions inline the same lock/drain/reset/unlock sequence as close before DeleteCriticalSection", '
                        '"the typed destructor retains that sequence directly; the original instructions do not establish how the source factored the shared cleanup"], '
                        '"reason": "The original full destructor belongs to the real NetFifo owner: it drains the queue under its lock, resets the queue, unlocks, then destroys the lock."}')
    assert candidate_binding == expected_binding, "NetFifo destructor interpretation changed"
    baseline_binding_index = baseline_source.rfind(
        binding_marker, 0, baseline_source.index("NetFifo::~NetFifo()"))
    baseline_binding_end = baseline_source.index("\n", baseline_binding_index)
    baseline_binding = baseline_source[baseline_binding_index:baseline_binding_end]
    expected_destructor = baseline_close.replace(
        "void NetFifo::close()", "NetFifo::~NetFifo()", 1)
    closing_brace = expected_destructor.rfind("}")
    expected_destructor = (expected_destructor[:closing_brace] +
                           "    DeleteCriticalSection(&lock_);\n" +
                           expected_destructor[closing_brace:])
    assert compact(candidate_destructor) == compact(expected_destructor), \
        "destructor drain sequence or lock-destruction ordering changed"

    import_annotation = "// RECOVERY_IMPORT: v1 __imp__DeleteCriticalSection@4 KERNEL32.dll name:DeleteCriticalSection\n\n"
    assert header_text.count(import_annotation) == 1, "DeleteCriticalSection import metadata changed"
    projected_header = header_text.replace(import_annotation, "", 1)
    assert projected_header == baseline_header, \
        "NetFifo header changed outside the exact import projection"

    assert text.count(candidate_destructor) == 1
    projected = text.replace(candidate_destructor, baseline_destructor, 1)
    assert projected.count(candidate_binding) == 1
    projected = projected.replace(candidate_binding, baseline_binding, 1)
    return projected


def test_historical_projection_preserves_unrelated_text_and_claims():
    relative = "src/net_fifo.cpp"
    text = (ROOT / relative).read_text()
    baseline = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT).decode()
    projected = historical_netfifo_structure_view(
        historical_core_claim_view(text, relative), relative)
    assert projected == baseline
    mutant = text.replace("this->count_ += 1;", "this->count_ += 2;", 1)
    assert mutant != text
    projected_mutant = historical_netfifo_structure_view(
        historical_core_claim_view(mutant, relative), relative)
    assert projected_mutant != baseline
    rows = read_text(text, ROOT / relative)
    unrelated = next(row for row in rows if row.address == 0x6339C0)
    assert unrelated.byte_exact
    mutant = write_bytes(text.encode(), [replace(unrelated, byte_exact=False)]).decode()
    projected_mutant = historical_netfifo_structure_view(
        historical_core_claim_view(mutant, relative), relative)
    assert projected_mutant != baseline


@pytest.mark.parametrize("relative", [
    "src/alpha.cpp", "src/alphanet.cpp", "src/movie_media.cpp",
    "src/net_class.cpp", "src/net_fifo.cpp", "src/netdaemon.cpp",
    "src/shared_word.cpp",
])
def test_only_native_promotions_preserve_complete_source(relative):
    path = ROOT / relative
    old = subprocess.check_output(["git", "show", BASE + ":" + relative], cwd=ROOT)
    before = read_text(old.decode(), path)
    after = read_text(path.read_text(), path)
    promoted = [replace(row, byte_exact=True, semantic=False)
                for row in before if row.address in SELECTED]
    assert promoted
    assert all(not row.byte_exact for row in before if row.address in SELECTED)
    current = path.read_text()
    if relative == "src/net_fifo.cpp":
        current = historical_netfifo_structure_view(current, relative)
    assert write_bytes(old, promoted) == current.encode()
    after = read_text(current, path)
    assert len(before) == len(after)
    for original, recorded in zip(before, after):
        expected = (replace(original, byte_exact=True, semantic=False)
                    if original.address in SELECTED else original)
        for field in fields(original):
            if field.name != "region":
                assert getattr(recorded, field.name) == getattr(expected, field.name)


@pytest.mark.parametrize("damage", ["dtor-body", "dtor-binding", "dtor-claim",
                                     "import",
                                     "header-outside", "close", "outside"])
def test_netfifo_direct_destructor_bridge_rejects_unapproved_changes(damage):
    relative = "src/net_fifo.cpp"
    source = (ROOT / relative).read_text()
    header = (ROOT / "src/net_fifo.h").read_text()
    original_source, original_header = source, header
    if damage == "dtor-claim":
        old_source = subprocess.check_output(
            ["git", "show", NETFIFO_STRUCTURE_BASE + ":src/net_fifo.cpp"],
            cwd=ROOT, text=True)
        row = next(row for row in read_text(source, ROOT / relative)
                   if row.address == 0x6339E0)
        original = next(row for row in read_text(old_source, ROOT / relative)
                        if row.address == 0x6339E0)
        assert row.byte_exact is True and row.semantic is False
        source = write_bytes(source.encode(), [replace(
            row, byte_exact=original.byte_exact, semantic=original.semantic)]).decode()
    if damage == "dtor-body":
        source = source.replace("DeleteCriticalSection(&lock_);",
                                "DeleteCriticalSection(0);", 1)
    elif damage == "dtor-binding":
        source = source.replace("retains that sequence directly",
                                "does not retain that sequence directly", 1)
    elif damage == "import":
        header = header.replace("name:DeleteCriticalSection", "name:Changed", 1)
    elif damage == "header-outside":
        header = header.replace("uint32_t count_;", "uint32_t changed_count_;", 1)
    elif damage == "close":
        source = source.replace("void NetFifo::close() {\n  EnterCriticalSection",
                                "void NetFifo::close() {\n  LeaveCriticalSection", 1)
    elif damage == "outside":
        source = source.replace("this->count_ += 1;", "this->count_ += 2;", 1)
    assert source != original_source or header != original_header, "damage anchor did not mutate source"
    with pytest.raises(AssertionError):
        projected = historical_netfifo_structure_view(
            source, relative, header_text=header)
        baseline = subprocess.check_output(
            ["git", "show", NETFIFO_STRUCTURE_BASE + ":src/net_fifo.cpp"],
            cwd=ROOT, text=True)
        assert projected == baseline
