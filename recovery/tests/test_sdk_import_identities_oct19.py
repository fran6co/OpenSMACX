"""Source/pinned SDK identities and bounded resolver refusal controls.

The synthetic ordinary-external fixture tests only resolver decisions. Fresh
application COFF checks separately establish the actual imports/operands; neither
test establishes the selected candidate DLL version, ABI or runtime behavior.
"""
from dataclasses import replace
import hashlib
import os
from pathlib import Path
from types import SimpleNamespace

import pytest

from match_recovery.import_source import import_targets, read_imports
from match_recovery.pe_coff import PeImage


ROOT = Path(__file__).resolve().parents[2]
PIN = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
# Exact exported names/stack-byte suffixes, not original application functions.
DEPENDENCIES = '''
GetWindowLongA@8 USER32.dll
DefWindowProcA@16 USER32.dll
DispatchMessageA@4 USER32.dll
TranslateMessage@4 USER32.dll
GetSystemMetrics@4 USER32.dll
ShowWindow@8 USER32.dll
BeginPaint@8 USER32.dll
EndPaint@8 USER32.dll
EnumThreadWindows@12 USER32.dll
GetCurrentThreadId@0 KERNEL32.dll
SetTimer@16 USER32.dll
timeSetEvent@20 WINMM.dll
CloseHandle@4 KERNEL32.dll
UnmapViewOfFile@4 KERNEL32.dll
CreateFileA@28 KERNEL32.dll
CreateFileMappingA@24 KERNEL32.dll
GetFileSize@8 KERNEL32.dll
MapViewOfFile@20 KERNEL32.dll
SetEndOfFile@4 KERNEL32.dll
SetFilePointer@16 KERNEL32.dll
DeleteObject@4 GDI32.dll
GetStockObject@4 GDI32.dll
GetTextExtentPoint32A@16 GDI32.dll
RemoveFontResourceA@4 GDI32.dll
SelectObject@8 GDI32.dll
DeleteDC@4 GDI32.dll
FreeLibrary@4 KERNEL32.dll
SetRect@20 USER32.dll
CreateRectRgnIndirect@4 GDI32.dll
SelectClipRgn@8 GDI32.dll
SetDIBColorTable@16 GDI32.dll
SetTextColor@8 GDI32.dll
InterlockedExchange@8 KERNEL32.dll
SetThreadPriority@8 KERNEL32.dll
Sleep@4 KERNEL32.dll
mmioClose@8 WINMM.dll
FindClose@4 KERNEL32.dll
FindFirstFileA@8 KERNEL32.dll
GetDC@4 USER32.dll
ReleaseDC@8 USER32.dll
DestroyCursor@4 USER32.dll
GetParent@4 USER32.dll
SetParent@8 USER32.dll
PtInRect@12 USER32.dll
mciSendCommandA@16 WINMM.dll
mmioDescend@16 WINMM.dll
mmioSeek@12 WINMM.dll
'''
CASES = tuple(tuple(line.split()) for line in DEPENDENCIES.splitlines() if line.strip())


@pytest.fixture(scope='module')
def bindings():
    return {binding.symbol: binding for binding in read_imports(ROOT)}


@pytest.fixture(scope='module')
def image():
    path = Path(os.environ.get('OPENSMACX_IMAGE', ROOT / '.opensmacx/game/terranx_original.exe'))
    if not path.is_file():
        pytest.skip('pinned original image required for import identity controls')
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == PIN
    return PeImage(data, PIN)


def _ordinary_reference(symbol, *, storage=2, section=0, value=0):
    return SimpleNamespace(symbols={1: SimpleNamespace(name=symbol, storage=storage,
        section=section, value=value)},
        sections=[SimpleNamespace(relocations=[(0, 1, 6)])])


@pytest.mark.parametrize('decorated,module', CASES)
def test_actual_source_keeps_explicit_dependency_identity(bindings, decorated, module):
    binding = bindings['__imp__' + decorated]
    assert binding.module == module
    assert binding.name == decorated.split('@')[0]
    assert binding.ordinal is None


@pytest.mark.parametrize('decorated,module', CASES)
def test_pinned_import_resolution_and_wrong_identity_refusals(bindings, image, decorated, module):
    binding = bindings['__imp__' + decorated]
    original, = (item for item in image.imports()
        if item.module.lower() == module.lower() and item.name == binding.name
        and item.ordinal is None)
    objects = {'bounded-control': _ordinary_reference(binding.symbol)}
    targets, reports = import_targets([binding], objects, image)
    assert targets == {binding.symbol: original.address}
    assert reports[0]['identity'] == 'pass'
    assert reports[0]['link_selection'] == 'unknown'
    for damaged in (replace(binding, module='not-a-pinned-dependency.dll'),
                    replace(binding, name='not_a_pinned_export')):
        targets, reports = import_targets([damaged], objects, image)
        assert targets == {}
        assert reports[0]['identity'] == reports[0]['link_selection'] == 'unknown'
    # A definition, weak binding or common allocation is not an SDK slot.
    for change in ({'section': 1}, {'storage': 105}, {'value': 4}):
        targets, reports = import_targets([binding],
            {'bounded-control': _ordinary_reference(binding.symbol, **change)}, image)
        assert targets == {}
        assert reports[0]['identity'] == reports[0]['link_selection'] == 'unknown'
