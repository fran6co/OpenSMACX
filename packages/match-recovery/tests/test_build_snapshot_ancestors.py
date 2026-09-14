"""Ancestor-selection equivalence; no compiler or configured provider calls."""

from pathlib import Path, PurePosixPath, PureWindowsPath
import os

import pytest

from match_recovery import build_check
from test_build_check import project, write_config


def original(path, root):
    return tuple(parent for parent in path.parents if parent.is_relative_to(root))


@pytest.mark.parametrize('path,root', [
    ('/repo/src/sub/file.cc', '/repo'), ('/repo/file.cc', '/repo'),
    ('/repo', '/repo'), ('/repo-other/file.cc', '/repo'),
    ('/repo/src/file.cc', '/'), ('/repo/src/file.cc', '.'),
    ('repo/src/file.cc', 'repo'), ('repo/src/file.cc', '.'),
    ('src/file.cc', 'src'), ('src/file.cc', '.'), ('.', '.'),
    ('repo/../file.cc', 'repo'), ('repo/../file.cc', 'repo/..'),
    ('/a/b/c/d/e/repo/src/nested/file.cc', '/a/b/c/d/e/repo'),
    ('/file.cc', '/'), ('file.cc', '.'), ('../file.cc', '..'),
    ('repo/src/file.cc', 'repo/src/file.cc'), ('/repo/file.cc', 'repo')])
def test_posix_exact_ancestor_sequence_for_all_root_forms(path, root):
    path, root = PurePosixPath(path), PurePosixPath(root)
    assert tuple(build_check._input_parents(path, root)) == original(path, root)


@pytest.mark.parametrize('path,root', [
    ('C:/repo/src/file.cc', 'c:/REPO'), ('C:/repo/file.cc', 'D:/repo'),
    ('C:/repo/file.cc', 'C:/'), ('C:repo/file.cc', 'C:repo'),
    ('//server/share/repo/file.cc', '//SERVER/SHARE/repo'),
    ('repo/src/file.cc', '.'), ('C:/repo/file.cc', '.')])
def test_windows_drive_case_and_unc_sequences_are_unchanged(path, root):
    path, root = PureWindowsPath(path), PureWindowsPath(root)
    assert tuple(build_check._input_parents(path, root)) == original(path, root)


@pytest.mark.parametrize('form', ['absolute', 'absolute_dot', 'relative', 'dot', 'parent_relative'])
def test_snapshot_result_or_refusal_matches_original_for_root_forms(project, monkeypatch, form):
    settings = build_check.Build.read(project)
    if form.startswith('absolute'):
        root = project if form == 'absolute' else project / '.'
    else:
        monkeypatch.chdir(project if form == 'dot' else project.parent)
        root = Path('.') if form == 'dot' else Path(project.name)
        if form == 'parent_relative':
            root = Path(project.name) / 'code' / '..'
    def outcome():
        try:
            return ('value', settings.snapshot(root))
        except (OSError, ValueError) as error:
            return ('refusal', type(error), str(error))
    actual = outcome()
    monkeypatch.setattr(build_check, '_input_parents', original)
    assert outcome() == actual


@pytest.mark.parametrize('damage', ['none', 'inside_leaf_alias', 'outside_leaf_alias', 'dangling_leaf',
    'directory_alias', 'input_root_alias', 'missing_source', 'nonregular_source',
    'versioned_alias', 'versioned_hardlink', 'versioned_missing'])
def test_snapshot_alias_canonical_versioned_rules_match_original(project, monkeypatch, damage):
    extra = project / 'support'
    extra.mkdir()
    alias = extra / 'entry'
    if damage in ('inside_leaf_alias', 'outside_leaf_alias', 'dangling_leaf', 'directory_alias'):
        target = ('../code/main.c' if damage == 'inside_leaf_alias' else
                  '/etc/hosts' if damage == 'outside_leaf_alias' else '../absent')
        if damage == 'directory_alias':
            (extra / 'alias').symlink_to(project / 'code', target_is_directory=True)
            write_config(project, inputs=['support/alias/main.c'])
        else:
            alias.symlink_to(target)
            write_config(project, inputs=['support/entry'])
    elif damage == 'input_root_alias':
        (project / 'code').rename(project / 'moved')
        (project / 'code').symlink_to(project / 'moved', target_is_directory=True)
    elif damage == 'missing_source':
        (project / 'code').rename(project / 'moved')
    elif damage == 'nonregular_source':
        os.mkfifo(project / 'code/fifo')
    elif damage.startswith('versioned_'):
        graph = project / 'BuildGraph.txt'
        if damage == 'versioned_alias':
            graph.symlink_to(project / 'code/main.c')
        elif damage == 'versioned_hardlink':
            os.link(project / 'code/main.c', graph)
        write_config(project, versioned_inputs=['BuildGraph.txt'])
    settings = build_check.Build.read(project)
    def outcome():
        try:
            return ('value', settings.snapshot(project))
        except (OSError, ValueError) as error:
            return ('refusal', type(error), str(error))
    actual = outcome()
    monkeypatch.setattr(build_check, '_input_parents', original)
    assert outcome() == actual
    assert (actual[0] == 'value') is (damage in ('none', 'inside_leaf_alias'))


def test_helper_performs_no_filesystem_observations_and_returns_same_check_order(tmp_path, monkeypatch):
    root = tmp_path / 'deep/replica/root'
    path = root / 'src/sub/file.cc'
    expected = original(path, root)
    def forbidden(*args, **kwargs):
        pytest.fail('ancestor selection must remain purely lexical')
    monkeypatch.setattr(Path, 'stat', forbidden)
    monkeypatch.setattr(Path, 'lstat', forbidden)
    monkeypatch.setattr(Path, 'resolve', forbidden)
    assert tuple(build_check._input_parents(path, root)) == expected


def test_no_cross_snapshot_ancestor_memo_can_hide_a_new_alias(project):
    extra = project / 'support'
    (extra / 'nested').mkdir(parents=True)
    (extra / 'nested/input.h').write_text('observed bytes')
    write_config(project, inputs=['support/nested/input.h'])
    settings = build_check.Build.read(project)
    settings.snapshot(project)
    (extra / 'nested').rename(extra / 'moved')
    (extra / 'nested').symlink_to(extra / 'moved', target_is_directory=True)
    with pytest.raises(ValueError, match='symlinked build input'):
        settings.snapshot(project)
