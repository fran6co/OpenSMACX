"""Actual-source forwarding/order controls, not native ABI or game proof."""
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

from recovery.tests.test_pact_ascent import definition

ROOT = Path(__file__).resolve().parents[2]


def test_actual_source_and_mutations():
    buffer = (ROOT / 'src/buffer.cpp').read_text()
    graphic = (ROOT / 'src/graphicwin.cpp').read_text()
    header = (ROOT / 'src/buffer.h').read_text()
    match = re.search(r'struct PcxInputView\s*\{[^}]*\};', header)
    assert match
    bodies = match.group() + '\n' + '\n'.join(
        definition(text, re.escape(signature)) for text, signature in (
            (buffer, 'int Buffer::load_pcx_from_source('),
            (graphic, 'int GraphicWin::probe_image_file('),
            (graphic, 'int GraphicWin::load_pcx_from_source('),
            (graphic, 'int GraphicWin::resize_screen_and_sync_palette('),
        ))
    fixture = Path(__file__).with_name('graphicwin_buffer_input_control.cpp').read_text()
    token = '// Candidate source bodies are appended here by run_controls.py.'
    assert fixture.count(token) == 1
    source = fixture.replace(token, bodies)
    mutations = (
        ('return 3;', 'return 30;'),
        ('return 7;', 'return 70;'),
        ('return Buffer::probe_image_file(filename, palette, color_base, color_count);', 'return 0;'),
        ('return Buffer::load_pcx_from_source(source, palette, color_base,\n                                      color_count);', 'return 0;'),
        ('Win::move(0, 0);\n  Buffer::sync_to_palette(PaletteActive);', 'Buffer::sync_to_palette(PaletteActive);\n  Win::move(0, 0);'),
    )
    compilers = [compiler for name in ('g++', 'clang++') if (compiler := shutil.which(name))]
    assert compilers, 'host compiler required'
    for compiler in compilers:
        with tempfile.TemporaryDirectory(prefix='osmx-gw-buffer-') as directory:
            cpp, binary = Path(directory) / 'control.cpp', Path(directory) / 'control'
            variants = [source]
            for old, new in mutations:
                assert source.count(old) == 1
                variants.append(source.replace(old, new, 1))
            for index, variant in enumerate(variants):
                cpp.write_text(variant)
                built = subprocess.run([compiler, '-std=c++98', '-Wall', '-Wextra',
                                        '-Werror', '-Wno-unused-parameter', str(cpp),
                                        '-o', str(binary)], capture_output=True, text=True, timeout=45)
                assert built.returncode == 0, built.stdout + built.stderr
                run = subprocess.run([str(binary)], capture_output=True, text=True, timeout=15)
                assert (run.returncode == 0) == (index == 0), (compiler, index, run.stdout, run.stderr)
