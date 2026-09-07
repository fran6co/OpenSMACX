"""Source publication failure controls; no proprietary compiler artifacts."""

from dataclasses import replace
import os

import pytest

from match_recovery.source import read_file, Tier
from match_recovery.source.asm import AsmComparison
from match_recovery.source import publication
from match_recovery.source.recording import record_claims
from match_recovery.source.writer import write_bytes


@pytest.fixture
def inputs(tmp_path):
    paths = [tmp_path / "first.cpp", tmp_path / "second.cpp"]
    for index, path in enumerate(paths, 1):
        path.write_bytes(f"// ORIGINAL: 0x0040{index}000 _f{index} 0x0040{index}000-0x0040{index}001\n"
                         f"// TRIED: old spelling;\n//   wrapped evidence\nvoid f{index}() {{}}\n".encode())
    return paths, {path: path.read_bytes() for path in paths}, tmp_path / "staging"


def publish(inputs, **kwargs):
    paths, expected, staging = inputs
    return publication.publish(expected, {path: b"changed\n" for path in paths},
                               staging_directory=staging, validate=lambda: None, **kwargs)


def record(inputs, *, results=None, measure=None, validate=lambda: None, **kwargs):
    paths, _, staging = inputs
    selected = [read_file(path)[0] for path in paths]
    return record_claims(selected, measure=measure or (lambda: list(zip(selected, results or [
        AsmComparison(verdict=Tier.BYTE_EXACT) for item in selected]))),
        validate=validate, staging_directory=staging, **kwargs)


def test_publishes_a_batch_without_leaving_temporary_files(inputs):
    paths, _, staging = inputs
    paths[0].chmod(0o755)
    assert publish(inputs) == tuple(paths)
    assert all(path.read_bytes() == b"changed\n" for path in paths)
    assert paths[0].stat().st_mode & 0o777 == 0o755
    assert not list(staging.iterdir())


def test_noop_preserves_inode_and_timestamp(inputs):
    paths, expected, staging = inputs
    before = [(path.stat().st_ino, path.stat().st_mtime_ns) for path in paths]
    assert publication.publish(expected, expected, validate=lambda: None, staging_directory=staging) == ()
    assert [(path.stat().st_ino, path.stat().st_mtime_ns) for path in paths] == before
    assert not staging.exists()


def test_rechecks_complete_inputs_after_staging(inputs):
    paths, expected, staging = inputs
    calls = []
    def validate():
        calls.append(1)
        if len(calls) == 2:
            raise ValueError("header changed")
    with pytest.raises(ValueError, match="header changed"):
        publication.publish(expected, {path: b"new" for path in paths},
                            validate=validate, staging_directory=staging)
    assert all(path.read_bytes() == expected[path] for path in paths)
    assert not list(staging.iterdir())


def test_second_replace_failure_restores_first_file(inputs, monkeypatch):
    paths, expected, staging = inputs
    real = os.replace
    def failing(source, destination):
        if destination == paths[1] and source.suffix == ".after":
            raise OSError("injected publication failure")
        real(source, destination)
    monkeypatch.setattr(publication.os, "replace", failing)
    with pytest.raises(OSError, match="injected publication failure"):
        publish(inputs)
    assert all(path.read_bytes() == expected[path] for path in paths)
    assert not list(staging.iterdir())


def test_rollback_preserves_subsequent_edit_and_original_backup(inputs, monkeypatch):
    paths, expected, staging = inputs
    real = os.replace
    def racing(source, destination):
        if destination == paths[1]:
            paths[0].write_bytes(b"someone else's work\n")
            raise OSError("second write failed")
        real(source, destination)
    monkeypatch.setattr(publication.os, "replace", racing)
    with pytest.raises(publication.PublicationError, match="backups retained") as error:
        publish(inputs)
    assert paths[0].read_bytes() == b"someone else's work\n"
    assert paths[1].read_bytes() == expected[paths[1]]
    backups = list(staging.iterdir())
    assert len(backups) == 1 and backups[0].read_bytes() == expected[paths[0]]
    assert str(backups[0]) in str(error.value)


def test_interrupt_immediately_after_replace_still_restores_originals(inputs, monkeypatch):
    paths, expected, staging = inputs
    real = os.replace
    def interrupting(source, destination):
        real(source, destination)
        if source.suffix == ".after":
            raise KeyboardInterrupt()
    monkeypatch.setattr(publication.os, "replace", interrupting)
    with pytest.raises(KeyboardInterrupt):
        publish(inputs)
    assert all(path.read_bytes() == expected[path] for path in paths)
    assert not list(staging.iterdir())


@pytest.mark.parametrize("change", ["content", "symlink", "hardlink", "mode", "missing"])
def test_stale_or_linked_outputs_are_refused(inputs, tmp_path, change):
    paths, expected, _ = inputs
    if change == "content":
        paths[0].write_bytes(b"another edit")
    elif change == "symlink":
        paths[0].unlink()
        paths[0].symlink_to(paths[1])
    elif change == "hardlink":
        os.link(paths[0], tmp_path / "alias.cpp")
    elif change == "mode":
        # Captured modes are checked again after validation too.
        def validate():
            paths[0].chmod(0o755)
        with pytest.raises(ValueError, match="changed"):
            publication.publish(expected, {paths[0]: b"new"}, validate=validate,
                                staging_directory=tmp_path / "staging")
        return
    else:
        paths[0].unlink()
    with pytest.raises((OSError, ValueError)):
        publish(inputs)
    assert paths[1].read_bytes() == expected[paths[1]]


def test_success_only_adds_claim_tokens_and_preserves_historical_notes(inputs):
    paths, expected, _ = inputs
    result = record(inputs)
    assert len(result.updated) == 2 and not result.regressed and not result.unresolved
    for path in paths:
        assert path.read_bytes().replace(b" BYTE_EXACT", b"") == expected[path]
        assert read_file(path)[0].ruled_out == ("old spelling; wrapped evidence",)


def test_unresolved_sibling_does_not_discard_proved_obligation(inputs):
    paths, expected, _ = inputs
    result = record(inputs, results=[AsmComparison(verdict=Tier.BYTE_EXACT), "symbol missing"])
    assert len(result.updated) == len(result.measured) == len(result.unresolved) == 1
    assert read_file(paths[0])[0].byte_exact
    assert paths[1].read_bytes() == expected[paths[1]]
    assert result.exit_code == 3


def test_regression_preserves_claim_and_historical_lever(inputs):
    paths, _, _ = inputs
    row = read_file(paths[0])[0]
    paths[0].write_bytes(write_bytes(paths[0].read_bytes(), [replace(row, byte_exact=True)]))
    paths[0].write_bytes(paths[0].read_bytes().replace(b"TRIED: old", b"LEVER: key  old"))
    before = paths[0].read_bytes()
    result = record(inputs, results=[AsmComparison(verdict=Tier.MISMATCH), AsmComparison(verdict=Tier.NO_COMPILE)])
    assert len(result.regressed) == 1 and not result.updated
    assert paths[0].read_bytes() == before
    assert result.exit_code == 1


def test_non_utf8_historical_notes_survive_recording(inputs):
    paths, _, _ = inputs
    paths[0].write_bytes(paths[0].read_bytes().replace(b"old spelling", b"old \xff spelling"))
    before = paths[0].read_bytes()
    result = record(inputs)
    assert result.exit_code == 0
    assert paths[0].read_bytes().replace(b" BYTE_EXACT", b"") == before


@pytest.mark.parametrize("tier", [Tier.NO_COMPILE, Tier.REFUSED, Tier.SHARED_TAIL])
def test_refusals_are_unmeasured_not_a_regression_or_success(inputs, tier):
    paths, _, _ = inputs
    path = paths[0]
    path.write_bytes(write_bytes(path.read_bytes(), [replace(read_file(path)[0], byte_exact=True)]))
    before = path.read_bytes()
    result = record(inputs, results=[AsmComparison(verdict=tier), AsmComparison(verdict=tier)])
    assert result.exit_code == 3 and len(result.unresolved) == 2 and not result.regressed
    assert path.read_bytes() == before


@pytest.mark.parametrize("corruption", ["missing", "extra", "duplicate", "altered", "invalid_tier"])
def test_measurement_must_cover_exactly_original_records(inputs, corruption):
    paths, expected, _ = inputs
    rows = [read_file(path)[0] for path in paths]
    results = [(row, AsmComparison(verdict=Tier.BYTE_EXACT)) for row in rows]
    if corruption == "missing":
        results.pop()
    elif corruption == "extra":
        results.append(results[0])
    elif corruption == "duplicate":
        results[1] = results[0]
    elif corruption == "invalid_tier":
        results[0] = (rows[0], AsmComparison(verdict="BYTE_EXACT"))
    else:
        results[0] = (replace(rows[0], name="different"), results[0][1])
    with pytest.raises(ValueError, match="measurement"):
        record(inputs, measure=lambda: results)
    assert all(path.read_bytes() == expected[path] for path in paths)


def test_source_change_during_measurement_is_preserved_not_stamped(inputs):
    paths, expected, _ = inputs
    def measure():
        rows = [read_file(path)[0] for path in paths]
        paths[1].write_bytes(b"another editor's work")
        return [(row, AsmComparison(verdict=Tier.BYTE_EXACT)) for row in rows]
    with pytest.raises(ValueError, match="changed"):
        record(inputs, measure=measure)
    assert paths[0].read_bytes() == expected[paths[0]]
    assert paths[1].read_bytes() == b"another editor's work"


@pytest.mark.parametrize("newline", [b"\n", b"\r\n", b"\r"])
@pytest.mark.parametrize("terminated", [True, False])
def test_byte_writer_preserves_encoding_newlines_and_final_termination(tmp_path, newline, terminated):
    path = tmp_path / "unit.cpp"
    raw = newline.join([b"// opaque note \xff", b"// ORIGINAL: 0x00401000 _f 0x00401000-0x00401001", b"void f() {}"])
    raw += newline if terminated else b""
    path.write_bytes(raw)
    row = read_file(path)[0]
    assert write_bytes(raw, [replace(row, byte_exact=True)]).replace(b" BYTE_EXACT", b"") == raw


def test_byte_writer_retains_mixed_newlines(inputs):
    paths, expected, _ = inputs
    path = paths[0]
    raw = expected[path].replace(b"\n", b"\r\n", 1)
    assert write_bytes(raw, [replace(read_file(path)[0], byte_exact=True)]).replace(b" BYTE_EXACT", b"") == raw
