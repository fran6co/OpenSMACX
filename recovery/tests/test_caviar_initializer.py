"""Synthetic controls of the proposed source-bound Caviar cache generator."""
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import struct
import subprocess

import pytest
from match_recovery.model import Span
from test_object_data import data_image

ROOT = Path(__file__).resolve().parents[2]


@pytest.fixture
def setup(tmp_path):
    spec = importlib.util.spec_from_file_location('caviar_hook', ROOT / 'recovery/terrain_data.py')
    hook = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(hook)
    hook._CAVIAR_PROJECTION_ROWS, hook._CAVIAR_PROJECTION_TABLE = Span(0x401000, 0x401018), Span(0x401018, 0x401024)
    hook._CAVIAR_PROJECTION_ROW_SIZE, hook._CAVIAR_PROJECTION_ROW_COUNT = 8, 3
    rows = bytes(range(24))
    raw = bytearray(data_image(rows + struct.pack('<III', 0x401000, 0x401008, 0x401010), absolute=24))
    struct.pack_into('<II', raw, 0x98 + 96 + 5 * 8, 0x1100, 16)
    struct.pack_into('<IIHHHH', raw, 768, 0x1000, 16, 0x3018, 0x301C, 0x3020, 0)
    root = tmp_path / 'project'
    (root / 'src').mkdir(parents=True)
    (root / 'recovery').mkdir()
    (root / 'recovery/project.json').write_text(json.dumps({'schema_version': 1, 'source': {'image_pin': 'src/IMAGE_PIN', 'address_space': 'pe:va', 'seed_roots': ['src/recovered', 'src/unrecovered'], 'symbol_format': 'msvc'}}))
    (root / '.gitignore').write_text('/build/\n')
    subprocess.run(['git', 'init', '-q', str(root)], check=True, capture_output=True)
    pin = hashlib.sha256(raw).hexdigest()
    (root / 'src/IMAGE_PIN').write_text('sha256 ' + pin + '\n')
    (root / 'src/caviar.cpp').write_text('// RECOVERY_OBJECT: v1 0x00401000 ' + hook._CAVIAR_PROJECTION_ROW_SYMBOL + '\nconst unsigned char rows[24] = {};\n')
    original = tmp_path / 'synthetic.exe'
    original.write_bytes(raw)
    output = root / 'build/recovery/preparation/terrain-data/caviar_angle_rows.inc'
    return hook, root, original, output, rows


def test_exact_idempotent_cache(setup):
    hook, root, original, output, rows = setup
    hook.caviar_projection_rows_initializer(root, original, output)
    assert bytes(int(value, 16) for value in re.findall(r'0x([0-9a-f]{2})', output.read_text())) == rows
    os.utime(output, ns=(1000000000, 1000000000))
    before = output.stat()
    hook.caviar_projection_rows_initializer(root, original, output)
    assert output.stat().st_mtime_ns == before.st_mtime_ns


@pytest.mark.parametrize('damage', ['pin', 'symbol', 'bounded', 'pointer', 'relocation', 'row_relocation', 'symlink'])
def test_refusal_preserves_cache(setup, damage):
    hook, root, original, output, _ = setup
    output.parent.mkdir(parents=True)
    output.write_text('previous cache\n')
    if damage == 'pin':
        (root / 'src/IMAGE_PIN').write_text('sha256 ' + '0' * 64 + '\n')
    elif damage == 'symbol':
        source = root / 'src/caviar.cpp'
        source.write_text(source.read_text().replace(hook._CAVIAR_PROJECTION_ROW_SYMBOL, '_wrong'))
    elif damage == 'bounded':
        (root / 'src/caviar.cpp').write_text('// RECOVERY_OBJECT: v2 0x00401000 0x00401018 ' + hook._CAVIAR_PROJECTION_ROW_SYMBOL + ' 0x00401000\n')
    elif damage == 'symlink':
        original_output = output.with_name('previous.inc')
        output.rename(original_output)
        output.symlink_to(original_output.name)
    else:
        raw = bytearray(original.read_bytes())
        if damage == 'pointer':
            struct.pack_into('<I', raw, 512 + 28, 0x401001)
        elif damage == 'relocation':
            struct.pack_into('<H', raw, 778, 0)
        else:
            struct.pack_into('<H', raw, 778, 0x3004)
        original.write_bytes(raw)
        (root / 'src/IMAGE_PIN').write_text('sha256 ' + hashlib.sha256(raw).hexdigest() + '\n')
    with pytest.raises(ValueError):
        hook.caviar_projection_rows_initializer(root, original, output)
    assert output.read_text() == 'previous cache\n'
