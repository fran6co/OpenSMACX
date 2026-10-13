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


@pytest.mark.parametrize('bit', [1, 2])
def test_point_light_signed_start_only_window(tmp_path, hook, bit):
    payload = bytes((index * 37 + 19) % 256 for index in range(113))
    root, original, unused = project(
        tmp_path, data_image(payload + b'neighbor'), size=113,
        symbol=f'_AltLightBit{bit}', unit='mapwin.cpp', bounded=False)
    output = unused.with_name(f'terrain_point_light_bit{bit}.inc')
    hook.point_light_initializer(root, original, output)
    values = [int(x) for x in re.findall(r'-?\d+', output.read_text().split('\n', 1)[1])]
    assert len(values) == 113 and bytes(x & 255 for x in values) == payload
    assert min(values) < 0 and max(values) > 0
    before = output.stat().st_mtime_ns
    hook.point_light_initializer(root, original, output)
    assert output.stat().st_mtime_ns == before
    original.write_bytes(data_image(bytes(113)))
    with pytest.raises(ValueError):
        hook.point_light_initializer(root, original, output)


def test_point_light_refuses_allocation_extent_claim(tmp_path, hook):
    root, original, unused = project(
        tmp_path, data_image(bytes(113)), size=113,
        symbol='_AltLightBit1', unit='mapwin.cpp', bounded=True)
    with pytest.raises(ValueError, match='start-only'):
        hook.point_light_initializer(root, original, unused.with_name('terrain_point_light_bit1.inc'))


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


LANDMARKS = [
    ('terrain_mount_planet.inc', '_MountPlanetTable'),
    ('terrain_sunny_mesa.inc', '_SunnyMesaTable'),
    ('terrain_garland_crater.inc', '_GarlandCraterTable'),
]
PAIR_PAYLOAD = bytes((index * 71 + 128) % 256 for index in range(98))


def landmark_project(tmp_path, filename, expected_symbol, **options):
    defaults = dict(raw=data_image(PAIR_PAYLOAD + b'neighbor'), size=98,
                    symbol=expected_symbol, unit='mapwin.cpp')
    defaults.update(options)
    root, original, output = project(tmp_path, **defaults)
    owner = root / 'src' / defaults['unit']
    text = owner.read_text().split('\n', 1)[0]
    owner.write_text(text + '\nstruct LandmarkTextureChoice { signed char texture, rotation; };\n'
                     + 'static const LandmarkTextureChoice ' + defaults['symbol'].lstrip('_')
                     + '[49] = {};\n')
    return root, original, output.with_name(filename)


@pytest.mark.parametrize('filename,symbol', LANDMARKS)
def test_signed_landmark_pairs_are_source_bound_and_cached(tmp_path, hook, filename, symbol):
    root, original, output = landmark_project(tmp_path, filename, symbol)
    hook.terrain_landmark_initializer(root, original, output)
    rows = re.findall(r'\{(-?\d+), (-?\d+)\},', output.read_text())
    assert len(rows) == 49
    actual = [int(value) for pair in rows for value in pair]
    assert actual == [value if value < 128 else value - 256 for value in PAIR_PAYLOAD]
    assert all(-128 <= value <= 127 for value in actual)
    assert bytes(value & 255 for value in actual) == PAIR_PAYLOAD
    before = output.stat()
    hook.terrain_landmark_initializer(root, original, output)
    assert output.stat().st_mtime_ns == before.st_mtime_ns
    output.write_text('stale table\n')
    hook.terrain_landmark_initializer(root, original, output)
    assert len(re.findall(r'\{', output.read_text())) == 49


@pytest.mark.parametrize('options', [
    {'size': 18}, {'size': 97}, {'size': 99}, {'size': 100},
    {'bounded': False}, {'unit': 'game.cpp'}, {'symbol': '_AnotherTable'},
])
def test_landmark_owner_extent_cannot_be_substituted(tmp_path, hook, options):
    filename, symbol = LANDMARKS[0]
    root, original, output = landmark_project(tmp_path, filename, symbol, **options)
    output.parent.mkdir(parents=True)
    output.write_text('previous cache\n')
    with pytest.raises(ValueError):
        hook.terrain_landmark_initializer(root, original, output)
    assert output.read_text() == 'previous cache\n'


@pytest.mark.parametrize('damage', ['pin', 'relocation', 'outside', 'unknown_name'])
def test_landmark_input_and_destination_refusals(tmp_path, hook, damage):
    filename, symbol = LANDMARKS[1]
    options = {'raw': data_image(PAIR_PAYLOAD, absolute=31)} if damage == 'relocation' else {}
    root, original, output = landmark_project(tmp_path, filename, symbol, **options)
    if damage == 'pin':
        (root / 'src/IMAGE_PIN').write_text('sha256 ' + '0' * 64 + '\n')
    elif damage == 'outside':
        output = tmp_path / filename
    elif damage == 'unknown_name':
        output = output.with_name('not_a_landmark.inc')
    with pytest.raises(ValueError):
        hook.terrain_landmark_initializer(root, original, output)
    assert not output.exists()


def test_configured_terrain_generation_requires_every_bound_table(tmp_path, hook):
    # Keep every reviewed generator input in one synthetic data section whose
    # RVA covers the newly projected fixed-address gaps as well as the older
    # configured tables. This is a source-bound fixture, not a claim about the
    # pinned executable's aggregate allocation.
    base_va = 0x685000
    section_rva = base_va - 0x400000
    payload_offset = 512
    payload_size = 0x3000
    payload = bytearray(payload_size)

    def place(address, values):
        start = address - base_va
        payload[start:start + len(values)] = values

    place(0x685A98, PAIR_PAYLOAD)
    place(0x685AFC, PAIR_PAYLOAD)
    place(0x685BD4, PAIR_PAYLOAD)
    place(0x685B60, bytes((index * 19 + 5) & 0xFF for index in range(113)))
    for _, address, size in (
            ('mount_mesa', 0x685AFA, 2), ('mesa_point', 0x685B5E, 2),
            ('point_crater', 0x685BD1, 3), ('crater_ocean', 0x685C36, 0x72)):
        place(address, bytes(((address + i) * 7) & 0xFF for i in range(size)))
    place(0x685CA8, bytes((index * 11 + 3) & 0xFF for index in range(128)))
    place(0x685D28, PAYLOAD)
    altitude_start = 0x686000
    for index, symbol in enumerate(hook._ALTITUDE_TABLES.values()):
        place(altitude_start + index * 44, bytes((index + i) & 0xFF for i in range(44)))
    direction_start = altitude_start + 44 * 3
    place(direction_start, bytes((i * 13 + 1) & 0xFF for i in range(245)))
    bonus_start = direction_start + 245
    for index, _ in enumerate(hook._LANDMARK_BONUS_TABLES.values()):
        place(bonus_start + index * 113,
              bytes((index * 23 + i) & 0xFF for i in range(113)))

    raw = bytearray(data_image(bytes(4)))
    raw.extend(bytes(payload_offset + payload_size - len(raw)))
    struct.pack_into('<II', raw, 0x178 + 8, payload_size, section_rva)
    struct.pack_into('<II', raw, 0x178 + 16, payload_size, payload_offset)
    struct.pack_into('<I', raw, 0x178 + 36, 0xC0000040)
    raw[payload_offset:payload_offset + payload_size] = payload
    root, original, output = project(tmp_path, bytes(raw))

    rows = ['struct LandmarkTextureChoice { signed char texture, rotation; };']
    parent = '?TerrainLandmarkStorageWindowData@@3UTerrainLandmarkStorageWindow@@A'
    for start, end, symbol, member in (
            (0x685A98, 0x685AFA, '_MountPlanetTable', 'mount_planet'),
            (0x685AFC, 0x685B5E, '_SunnyMesaTable', 'sunny_mesa'),
            (0x685B60, None, '_AltLightBit2', 'alt_light_bit2'),
            (0x685BD4, 0x685C36, '_GarlandCraterTable', 'garland_crater'),
            (0x685CA8, 0x685D28, '_OceanCornerUVs', 'ocean_corner_uvs')):
        if end is None:
            rows.append(f'// RECOVERY_OBJECT: v3 0x{start:08X} {symbol} member {parent} {member}')
        else:
            rows.append(f'// RECOVERY_OBJECT: v3 0x{start:08X} 0x{end:08X} {symbol} 0x004632D0 member {parent} {member}')
    rows.append('static const signed char TerrainLandmarkStorageWindowData[1] = {};')
    source = root / 'src/mapwin.cpp'
    source.write_text('\n'.join(rows) + '\n')
    game_rows = [f'// RECOVERY_OBJECT: v2 0x00685D28 0x00685D5D _TerrainCoastRemapSource 0x004CF7C0',
                 'static const unsigned char TerrainCoastRemapSource[53] = {};']
    (root / 'src/game.cpp').write_text('\n'.join(game_rows) + '\n')
    altitude_rows = []
    for index, symbol in enumerate(hook._ALTITUDE_TABLES.values()):
        start = altitude_start + index * 44
        altitude_rows.append(f'// RECOVERY_OBJECT: v2 0x{start:08X} '
                             f'0x{start + 44:08X} {symbol} 0x00401000')
    start = direction_start
    altitude_rows.append(f'// RECOVERY_OBJECT: v2 0x{start:08X} '
                         f'0x{start + 245:08X} ?AltDirectionTable@@3QBEB 0x00401000')
    for index, symbol in enumerate(hook._LANDMARK_BONUS_TABLES.values()):
        address = bonus_start + index * 113
        altitude_rows.append(f'// RECOVERY_OBJECT: v1 0x{address:08X} {symbol}')
    (root / 'src/map.cpp').write_text('\n'.join(altitude_rows) + '\n')
    hook.terrain_initializers(root, original, output.parent)
    assert {path.name for path in output.parent.iterdir()} == {
        'terrain_coast_remap.inc', *(filename for filename, _ in LANDMARKS),
        'terrain_landmark_gap_mount_mesa.inc',
        'terrain_landmark_gap_mesa_point_light.inc',
        'terrain_landmark_gap_point_light_crater.inc',
        'terrain_landmark_gap_crater_ocean.inc',
        *hook._ALTITUDE_TABLES, 'terrain_alt_direction.inc',
        *hook._LANDMARK_BONUS_TABLES}
    source.write_text(source.read_text().replace('_GarlandCraterTable', '_OtherTable'))
    # CMake must refuse even with all seven earlier cache files still present.
    with pytest.raises(ValueError):
        hook.terrain_initializers(root, original, output.parent)


@pytest.mark.parametrize('filename', [
    'terrain_altitude_default.inc', 'terrain_altitude_contour.inc',
    'terrain_altitude_natural.inc'])
def test_altitude_words_are_exact_unsigned_little_endian(tmp_path, hook, filename):
    words = [0, 1, 255, 256, 65535, 65536, 0x01020304,
             0x7fffffff, 0x80000000, 0xfffffffe, 0xffffffff]
    raw = data_image(struct.pack('<11I', *words))
    root, original, output = project(
        tmp_path, raw, size=44, unit='map.cpp',
        symbol=hook._ALTITUDE_TABLES[filename])
    output = output.with_name(filename)
    hook.terrain_altitude_initializer(root, original, output)
    actual = list(map(int, re.findall(r'(\d+)u', output.read_text())))
    assert actual == words
    before = output.stat().st_mtime_ns
    hook.terrain_altitude_initializer(root, original, output)
    assert output.stat().st_mtime_ns == before


@pytest.mark.parametrize('damage', ['pin', 'extent', 'relocation', 'unknown_name'])
def test_altitude_refusals_preserve_previous_cache(tmp_path, hook, damage):
    raw = data_image(bytes(44), absolute=4) if damage == 'relocation' else data_image(bytes(44))
    root, original, output = project(
        tmp_path, raw, size=43 if damage == 'extent' else 44,
        unit='map.cpp', symbol='?AltNatural@@3PAIA')
    output = output.with_name('terrain_altitude_natural.inc')
    if damage == 'pin':
        (root / 'src/IMAGE_PIN').write_text('sha256 ' + '0' * 64 + '\n')
    elif damage == 'unknown_name':
        output = output.with_name('unreviewed_altitude.inc')
    output.parent.mkdir(parents=True)
    output.write_text('previous cache\n')
    with pytest.raises(ValueError):
        hook.terrain_altitude_initializer(root, original, output)
    assert output.read_text() == 'previous cache\n'


def test_actual_product_altitude_arrays_use_generated_initializers(tmp_path):
    import shutil
    compiler = shutil.which('g++')
    if compiler is None:
        pytest.skip('host C++ compiler required')
    source = (ROOT / 'src/map.cpp').read_text()
    definitions = []
    for name, filename in [('AltNaturalDefault', 'terrain_altitude_default.inc'),
                           ('AltContour', 'terrain_altitude_contour.inc'),
                           ('AltNatural', 'terrain_altitude_natural.inc')]:
        definition, = re.findall(r'uint32_t ' + name + r'\[11\] = \{.*?\};', source, re.S)
        definitions.append(definition)
        values = list(range(11))
        (tmp_path / filename).write_text(','.join(f'{value}u' for value in values))
    harness = '#include <cassert>\ntypedef unsigned int uint32_t;\n' + '\n'.join(definitions)
    harness += '''
int main() {
    static_assert(sizeof(AltNatural) == 44, "real bounded array");
    static_assert(sizeof(AltNaturalDefault) == 44, "real default array");
    static_assert(sizeof(AltContour) == 44, "real contour array");
    for (int i=0; i<11; ++i) {
        assert(AltNatural[i] == static_cast<unsigned>(i));
        assert(AltNaturalDefault[i] == static_cast<unsigned>(i));
        assert(AltContour[i] == static_cast<unsigned>(i));
    }
    AltNatural[3] = 999;
    assert(AltNaturalDefault[3] == 3 && AltContour[3] == 3);
}
'''
    cpp = tmp_path / 'arrays.cpp'
    cpp.write_text(harness)
    exe = tmp_path / 'arrays'
    built = subprocess.run([compiler, '-std=c++11', str(cpp), '-o', str(exe)],
                           capture_output=True, text=True)
    assert built.returncode == 0, built.stderr
    assert subprocess.run([str(exe)]).returncode == 0

def test_graphicwin_palette_initializer_projects_only_start_bound_window(tmp_path, hook):
    payload = bytes((index * 29 + 7) % 256 for index in range(132))
    root, original, unused = project(
        tmp_path, data_image(payload + b"neighbor"), start=0x401000, size=132,
        symbol='?GraphicWinBlitPolynomialWords@@3PAIA', unit="graphicwin.cpp", bounded=False)
    output = unused.with_name("graphicwin_palette_polynomials.inc")
    hook.graphicwin_palette_initializer(root, original, output)
    values = [int(value) for value in
              re.findall(r"(?<![A-Za-z0-9_])([0-9]+)u", output.read_text())]
    expected = [int.from_bytes(payload[index:index + 4], "little")
                for index in range(0, 132, 4)]
    assert values == expected and len(values) == 33
    assert "Generated from pinned source-owned terrain data" in output.read_text()
    first = output.read_bytes()
    hook.graphicwin_palette_initializer(root, original, output)
    assert output.read_bytes() == first


def test_graphicwin_palette_initializer_refuses_a_bounded_extent(tmp_path, hook):
    payload = bytes((index * 29 + 7) % 256 for index in range(132))
    root, original, unused = project(
        tmp_path, data_image(payload + b"neighbor"), start=0x401000, size=132,
        symbol='?GraphicWinBlitPolynomialWords@@3PAIA', unit="graphicwin.cpp", bounded=True)
    output = unused.with_name("graphicwin_palette_polynomials.inc")
    with pytest.raises(ValueError, match="start-only"):
        hook.graphicwin_palette_initializer(root, original, output)
    assert not output.exists()
