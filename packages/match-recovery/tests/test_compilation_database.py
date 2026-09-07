"""Portable database parsing without a compiler, project adapter or shell."""

import json
import pytest

from match_recovery.compilation_database import entries, read_database, build_inputs


def test_paths_and_argv_are_resolved_without_retokenizing(tmp_path):
    args = ["compiler with spaces", "-DMESSAGE=hello world", "unit.cc"]
    entry, = entries(json.dumps([{"directory": "out", "file": "../code/unit.cc",
                                  "arguments": args}]).encode(), tmp_path)
    assert entry.file == tmp_path / "code/unit.cc"
    assert entry.directory == tmp_path / "out"
    assert entry.arguments == tuple(args) and entry.command is None
    # Identity parsing does not pretend the configured inputs currently exist.
    assert not entry.file.exists()


def test_command_strings_stay_opaque_and_multiple_recipes_stay_visible(tmp_path):
    rows = [{"directory": str(tmp_path), "file": "a.cc", "command": text}
            for text in ('compiler "a b"', 'compiler different')]
    result = entries(json.dumps(rows).encode(), tmp_path)
    assert len(result) == 2 and result[0].file == result[1].file
    assert [entry.command for entry in result] == [row["command"] for row in rows]
    database = tmp_path / "commands.json"
    database.write_text(json.dumps(rows))
    assert build_inputs(database) == {tmp_path / "a.cc"}


@pytest.mark.parametrize("data", [b'{}', b'[]', b'[NaN]',
    b'[{"file":"a","directory":".","command":"x","command":"y"}]',
    b'[{"file":"a","directory":".","arguments":["x"],"command":12}]',
    b'[{"file":"a","directory":".","arguments":[]}]',
    b'[{"file":"a","directory":".","arguments":["x",null]}]',
    b'[{"file":"a","arguments":["x"]}]',
    b'[{"file":"a","directory":"."}]'])
def test_malformed_database_is_not_an_empty_build(data, tmp_path):
    with pytest.raises(ValueError):
        entries(data, tmp_path)


def test_missing_database_is_explicit(tmp_path):
    with pytest.raises(ValueError, match="configure the build"):
        read_database(tmp_path / "absent.json")
