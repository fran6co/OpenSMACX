"""Synthetic controls for the actual configure-time terrain data hook.

No proprietary image bytes, native compilation or gameplay execution. Source
and PE parsing plus artifact_parent are the installed package's real APIs.
Git writes are confined to disposable pytest-owned synthetic repositories.
"""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess

import pytest

from test_object_data import data_image


ROOT = Path(__file__).resolve().parents[2]
PAYLOAD = bytes((index * 37 + 19) % 256 for index in range(53))


@pytest.fixture
def hook():
    spec = importlib.util.spec_from_file_location(
        'terrain_data_under_test', ROOT / 'recovery/terrain_data.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def git(root, *arguments):
    result = subprocess.run(['git', *arguments], cwd=root,
                            capture_output=True, text=True)
    assert result.returncode == 0, result.stderr
    return result


def project(tmp_path, raw=None, *, start=0x401000, size=53,
            symbol='_TerrainCoastRemapSource', unit='game.cpp', bounded=True):
    root = tmp_path / 'project'
    (root / 'src').mkdir(parents=True)
    (root / 'recovery').mkdir()
    (root / '.gitignore').write_text('/build/\n')
    git(root, 'init', '-q')
    (root / 'recovery/project.json').write_text(json.dumps({
        'schema_version': 1, 'source': {
            'image_pin': 'src/IMAGE_PIN', 'address_space': 'pe:va',
            'seed_roots': ['src/recovered', 'src/unrecovered'],
            'symbol_format': 'msvc'}}))
    raw = data_image(PAYLOAD + b'neighbor') if raw is None else raw
    original = tmp_path / 'synthetic.exe'
    original.write_bytes(raw)
    pin = hashlib.sha256(raw).hexdigest()
    (root / 'src/IMAGE_PIN').write_text(f'sha256 {pin}\n')
    binding = (f'// RECOVERY_OBJECT: v2 0x{start:08X} '
               f'0x{start + size:08X} {symbol} 0x00401000\n') if bounded else (
               f'// RECOVERY_OBJECT: v1 0x{start:08X} {symbol}\n')
    (root / 'src' / unit).write_text(
        binding + 'static const unsigned char TerrainCoastRemapSource[53] = {};\n')
    output = root / 'build/recovery/preparation/generated/terrain_coast_remap.inc'
    return root, original, output


def test_exact_source_bound_output_and_idempotence(tmp_path, hook):
    root, original, output = project(tmp_path)
    hook.terrain_coast_initializer(root, original, output)
    first = output.read_bytes()
    numbers = [int(value) for value in re.findall(r'\d+', output.read_text().split('\n', 1)[1])]
    assert bytes(numbers) == PAYLOAD and len(numbers) == 53
    assert len(output.read_text().splitlines()) == 5
    os.utime(output, ns=(1234567890000000000, 1234567890000000000))
    before = output.stat()
    hook.terrain_coast_initializer(root, original, output)
    after = output.stat()
    assert output.read_bytes() == first
    assert (after.st_ino, after.st_mtime_ns) == (before.st_ino, before.st_mtime_ns)
    # An incorrect untracked cache is regenerated, never treated as evidence.
    output.write_text('stale cache\n')
    hook.terrain_coast_initializer(root, original, output)
    assert output.read_bytes() == first


def test_span_location_comes_from_source_not_hardcoded_va(tmp_path, hook):
    raw = data_image(bytes(17) + PAYLOAD + b'neighbor')
    root, original, output = project(tmp_path, raw, start=0x401011)
    hook.terrain_coast_initializer(root, original, output)
    values = re.findall(r'\d+', output.read_text().split('\n', 1)[1])
    assert bytes(map(int, values)) == PAYLOAD


@pytest.mark.parametrize('damage', ['wrong_pin', 'malformed_pin', 'changed_image'])
def test_pin_refusal_preserves_existing_cache(tmp_path, hook, damage):
    root, original, output = project(tmp_path)
    output.parent.mkdir(parents=True)
    output.write_text('previous cache\n')
    if damage == 'wrong_pin':
        (root / 'src/IMAGE_PIN').write_text('sha256 ' + '0' * 64 + '\n')
    elif damage == 'malformed_pin':
        (root / 'src/IMAGE_PIN').write_text('no pin\n')
    else:
        original.write_bytes(original.read_bytes() + b'changed')
    with pytest.raises(ValueError):
        hook.terrain_coast_initializer(root, original, output)
    assert output.read_text() == 'previous cache\n'


@pytest.mark.parametrize('options', [
    {'size': 52}, {'size': 54}, {'bounded': False},
    {'start': 0x402000}, {'start': 0x4011F0},
    {'symbol': '_AnotherTable'}, {'unit': 'another.cpp'},
])
def test_missing_wrong_or_unbacked_source_interpretation(tmp_path, hook, options):
    root, original, output = project(tmp_path, **options)
    with pytest.raises(ValueError):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()


def test_virtual_zero_tail_is_not_materialized_as_backed_data(tmp_path, hook):
    raw = bytearray(data_image(PAYLOAD))
    # Keep a 512-byte virtual section but only 32 file-backed bytes.
    struct.pack_into('<I', raw, 0x178 + 16, 32)
    root, original, output = project(tmp_path, bytes(raw))
    with pytest.raises(ValueError, match='backed'):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.parametrize('flags', [0x60000020, 0xC0000060, 0xE0000040, 0x40000000])
def test_code_or_executable_data_cannot_be_exported(tmp_path, hook, flags):
    raw = bytearray(data_image(PAYLOAD))
    struct.pack_into('<I', raw, 0x178 + 36, flags)
    root, original, output = project(tmp_path, bytes(raw))
    with pytest.raises(ValueError):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.parametrize('offset', [0, 17, 51])
def test_relocation_inside_or_crossing_boundary_refuses(tmp_path, hook, offset):
    root, original, output = project(tmp_path, data_image(PAYLOAD, absolute=offset))
    with pytest.raises(ValueError, match='relocat'):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.parametrize('location', ['outside', 'source', 'wrong_name', 'dotdot'])
def test_output_scope_refusal(tmp_path, hook, location):
    root, original, output = project(tmp_path)
    if location == 'outside':
        output = tmp_path / 'terrain_coast_remap.inc'
    elif location == 'source':
        output = root / 'src/terrain_coast_remap.inc'
    elif location == 'wrong_name':
        output = output.with_name('another.inc')
    else:
        output = root / 'build/../terrain_coast_remap.inc'
    with pytest.raises(ValueError):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()


@pytest.mark.parametrize('kind', ['file_symlink', 'parent_symlink', 'hardlink', 'directory'])
def test_output_aliases_and_nonregular_targets_are_preserved(tmp_path, hook, kind):
    root, original, output = project(tmp_path)
    target = tmp_path / 'protected'
    target.write_text('protected contents\n')
    if kind == 'parent_symlink':
        (root / 'build').symlink_to(tmp_path, target_is_directory=True)
    else:
        output.parent.mkdir(parents=True)
        if kind == 'file_symlink':
            output.symlink_to(target)
        elif kind == 'hardlink':
            os.link(target, output)
        else:
            output.mkdir()
    with pytest.raises(ValueError):
        hook.terrain_coast_initializer(root, original, output)
    assert target.read_text() == 'protected contents\n'


@pytest.mark.parametrize('kind', ['not_ignored', 'tracked_output', 'tracked_neighbor'])
def test_real_git_artifact_protection(tmp_path, hook, kind):
    root, original, output = project(tmp_path)
    output.parent.mkdir(parents=True)
    if kind == 'not_ignored':
        (root / '.gitignore').write_text('')
    else:
        tracked = output if kind == 'tracked_output' else output.with_name('tracked.txt')
        tracked.write_text('versioned contents\n')
        git(root, 'add', '-f', '--', tracked.relative_to(root).as_posix())
    with pytest.raises(ValueError, match='Git-ignored'):
        hook.terrain_coast_initializer(root, original, output)
    if kind != 'not_ignored':
        assert tracked.read_text() == 'versioned contents\n'


@pytest.mark.parametrize('damage', ['image', 'pin', 'binding'])
def test_changed_authority_before_publication_refuses(tmp_path, hook, monkeypatch, damage):
    root, original, output = project(tmp_path)
    real = hook.PeImage

    def read_then_change(data, pin):
        # Validate the original snapshot through the real PE reader first;
        # only then change authority, so this exercises publication rechecks
        # rather than merely another initial digest-mismatch refusal.
        image = real(data, pin)
        if damage == 'image':
            original.write_bytes(original.read_bytes() + b'changed')
        elif damage == 'pin':
            (root / 'src/IMAGE_PIN').write_text('sha256 ' + '0' * 64 + '\n')
        else:
            source = root / 'src/game.cpp'
            source.write_text(source.read_text().replace('0x00401035', '0x00401034'))
        return image

    monkeypatch.setattr(hook, 'PeImage', read_then_change)
    with pytest.raises(ValueError, match='inputs changed'):
        hook.terrain_coast_initializer(root, original, output)
    assert not output.exists()
