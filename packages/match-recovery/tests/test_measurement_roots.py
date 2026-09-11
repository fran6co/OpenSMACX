"""Identical logical roots need one walk, without collapsing alias spellings."""

from dataclasses import replace
import hashlib
import os

from match_recovery.measurement import Measurement
from test_symbol_check import project


def test_identical_source_and_input_roots_are_walked_once_with_fresh_membership(project, monkeypatch):
    settings = replace(Measurement.read(project), input_roots=("code",))
    original, walked = os.walk, []

    def observed(path, *args, **kwargs):
        walked.append(path)
        return original(path, *args, **kwargs)

    monkeypatch.setattr(os, "walk", observed)
    first = settings.snapshot(project, {})
    assert walked == [project / "code"]
    directory = project / "code/.generated"
    directory.mkdir()
    header = directory / "new.inc"
    header.write_bytes(b"new header")
    second = settings.snapshot(project, {})
    assert walked == [project / "code", project / "code"]
    assert second != first
    assert second["inputs"]["code/.generated/new.inc"] == hashlib.sha256(b"new header").hexdigest()
    header.unlink()
    assert settings.snapshot(project, {}) == first


def test_distinct_alias_roots_keep_each_logical_input_name(project, monkeypatch):
    (project / "alias").symlink_to("code", target_is_directory=True)
    header = project / "code/input.inc"
    header.write_bytes(b"shared input")
    settings = replace(Measurement.read(project), input_roots=("code", "alias"))
    original, walked = os.walk, []

    def observed(path, *args, **kwargs):
        walked.append(path)
        return original(path, *args, **kwargs)

    monkeypatch.setattr(os, "walk", observed)
    report = settings.snapshot(project, {})
    assert walked == [project / "code", project / "alias"]
    expected = hashlib.sha256(b"shared input").hexdigest()
    assert report["inputs"]["code/input.inc"] == expected
    assert report["inputs"]["alias/input.inc"] == expected
