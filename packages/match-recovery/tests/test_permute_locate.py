"""`recover permute` locates a target body through its own ORIGINAL mapping."""
from pathlib import Path

import pytest

from match_recovery.source import read_text
from match_recovery.source_permute import _locate_body

TEXT = """\
// ORIGINAL: 0x00401000 ?info@@YAXPAXH@Z 0x00401000-0x00401010 FILE
void info(void *emit, int value) {
    value += 1;
}

// ORIGINAL: 0x00401020 ?info@@YAXH@Z 0x00401020-0x00401030
void info(int value) {
    info(0, value);
}
"""


def located(address):
    path = Path("/project/src/info.cpp")
    record, = [row for row in read_text(TEXT, path) if row.address == address]
    host, opening, closing, parameters = _locate_body(
        record, "src/info.cpp", {"src/info.cpp": TEXT}, "msvc", Path("/project"))
    return host, TEXT[opening:closing + 1], parameters


def test_overloaded_file_mapping_resolves_by_its_original_address():
    # The mapping's name is its mangled symbol; the overload is chosen by the
    # marker's own interval, never by the shared C++ name alone.
    assert located(0x00401000) == ("src/info.cpp", "{\n    value += 1;\n}", "void *emit, int value")


def test_overloaded_body_mapping_still_resolves():
    assert located(0x00401020) == ("src/info.cpp", "{\n    info(0, value);\n}", "int value")


def test_overload_without_its_own_interval_stays_refused():
    text = TEXT.replace("// ORIGINAL: 0x00401020 ?info@@YAXH@Z 0x00401020-0x00401030\n", "")
    path = Path("/project/src/info.cpp")
    record, = read_text(text, path)
    with pytest.raises(ValueError):
        _locate_body(record, "src/info.cpp", {"src/info.cpp": text}, "msvc", Path("/project"))


def test_trial_scoring_compiles_dependency_units_under_their_own_recipe(tmp_path):
    # Strict associated-data scoring also compiles the evidence closure: other
    # TUs under their own (for example unpooled /Og /Oi /Ot /Gs /Gy) recipe.
    from match_recovery.source_permute import _trial_object
    unit, other = tmp_path / "unit.cpp", tmp_path / "other.cpp"
    pooled, unpooled = ("/O2",), ("/Og", "/Oi", "/Ot", "/Gs", "/Gy")
    calls, cache = [], {}

    def dependency(path, arguments, trial):
        calls.append((path, arguments, trial))
        return b"other:" + " ".join(trial).encode()

    raws = {pooled: b"trial"}
    assert _trial_object(unit, raws, cache, dependency, unit, (), list(pooled)) == b"trial"
    for _ in range(2):
        assert _trial_object(unit, raws, cache, dependency, other, (), list(unpooled)) == b"other:/Og /Oi /Ot /Gs /Gy"
    assert calls == [(other.resolve(), (), unpooled)]
    # The target unit never borrows a dependency compile or another flag set.
    with pytest.raises(ValueError, match="compiler command failed"):
        _trial_object(unit, raws, cache, dependency, unit, (), list(unpooled))
