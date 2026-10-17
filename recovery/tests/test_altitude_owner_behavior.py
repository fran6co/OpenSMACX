"""Actual terrain source controls with synthetic tables, not ABI/game proof."""
from pathlib import Path
import hashlib
import os
import shutil
import subprocess
import pytest
from match_recovery.source import read_file
from match_recovery.source.reader import _extract_forward_text, _block_state_after
ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).with_name("altitude_owners")
COMPILERS = [c for c in ("g++", "clang++") if shutil.which(c)]

def actual(address):
    path = ROOT / "src/map.cpp"
    record = next(r for r in read_file(path) if r.address == address)
    lines = path.read_text().splitlines()
    block = False
    for line in lines[:record.line - 1]:
        block = _block_state_after(line, block)
    return _extract_forward_text(lines, record.line, record.name, in_block=block)

def execute(tmp_path, compiler, case, body):
    tmp_path.mkdir(parents=True, exist_ok=True)
    fixture = FIXTURES / ("alt_set.cpp" if case == "alt_set" else "control.cpp")
    source = tmp_path / "control.cpp"
    source.write_text(fixture.read_text().replace("/* ACTUAL_BODIES */", body))
    # Synthetic valid lookup rows distinguish row/corner indexing. They do not
    # claim original data; pinned initializer and native data checks are separate.
    values = [row if column == 0 else (row + column * 17) % 113
              for row in range(49) for column in range(5)]
    (tmp_path / "alt_direction_table.inc").write_text(",".join(map(str, values)))
    binary = tmp_path / "control"
    subprocess.run([compiler, "-std=c++98", "-Wall", "-Wextra", "-Werror",
                    "-fsanitize=undefined", "-fno-sanitize-recover=all", str(source),
                    "-o", str(binary)], check=True, capture_output=True, text=True)
    return subprocess.run([str(binary)], capture_output=True, timeout=10)

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case", ["alt_set", "delete_landmark"])
def test_actual_source_transitions(tmp_path, compiler, case):
    body = actual(0x591290) if case == "alt_set" else actual(0x592940) + "\n" + actual(0x5927D0)
    result = execute(tmp_path, compiler, case, body)
    assert result.returncode == 0, result.stderr

@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("case,before,after", [
    ("alt_set", "!has_fac_built(FAC_PRESSURE_DOME, base_id)", "has_fac_built(FAC_PRESSURE_DOME, base_id)"),
    ("delete_landmark", "if (old_flags == 0)", "if (old_flags != 0)"),
    ("delete_landmark", "window->view_active_ != 0", "window->view_active_ == 0"),
    ("delete_landmark", "AltBonusFlat[digits[i]] = 0", "AltBonusOcean[digits[i]] = 0"),
    ("delete_landmark", "static_cast<int32_t>(tile->bit2) >> 24", "tile->bit2 >> 24"),
    ("delete_landmark", "AltBonusMoist[digits[i]] = 0", "AltBonusMoist[digits[i]] = digits[4]"),
])
def test_damage_controls(tmp_path, compiler, case, before, after):
    body = actual(0x591290) if case == "alt_set" else actual(0x592940) + "\n" + actual(0x5927D0)
    positive = execute(tmp_path / 'positive', compiler, case, body)
    assert positive.returncode == 0, positive.stderr
    assert before in body
    result = execute(tmp_path / 'damaged', compiler, case, body.replace(before, after))
    assert result.returncode != 0


@pytest.mark.parametrize("compiler", COMPILERS)
@pytest.mark.parametrize("damage", ["early-return", "reversed-guard", "ungated-gameplay", "lost-low-bits", "missing-dirty"])
def test_zero_turn_damage_controls(tmp_path, compiler, damage):
    body = actual(0x591290)
    positive = execute(tmp_path / 'positive', compiler, 'alt_set', body)
    assert positive.returncode == 0, positive.stderr
    if damage == 'early-return':
        anchor = 'void __cdecl alt_set(int x, int y, int altitude) {'
        assert body.count(anchor) == 1
        body = body.replace(anchor, anchor + '\n    if (GameDataStateStorage.fields_.turn_current_num_ == 0) return;', 1)
    else:
        before, after = {
            'reversed-guard': ('GameDataStateStorage.fields_.turn_current_num_ != 0', 'GameDataStateStorage.fields_.turn_current_num_ == 0'),
            'ungated-gameplay': ('GameDataStateStorage.fields_.turn_current_num_ != 0', 'true'),
            'lost-low-bits': ('tile->climate & 0x1F', 'tile->climate & 0'),
            'missing-dirty': ('UnkBitfield1 |= 4;', 'UnkBitfield1 |= 0;'),
        }[damage]
        assert before in body
        body = body.replace(before, after, 1)
    damaged = execute(tmp_path / 'damaged', compiler, 'alt_set', body)
    assert damaged.returncode != 0, 'compiled mutation unexpectedly preserved the controls'


def test_pinned_zero_turn_branch_joins_climate_write_not_return():
    """Independent original instruction witness, not a check of candidate spelling."""
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    pin = '01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf'
    path = Path(os.environ.get('OPENSMACX_IMAGE', str(ROOT / '.opensmacx/game/terranx_original.exe')))
    if not path.is_file():
        pytest.skip('pinned original unavailable')
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == pin
    image = PeImage(data, pin)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    def listing(start, end):
        return {i.address: (i.mnemonic, i.op_str)
                for i in decoder.disasm(image.read(Span(start, end)), start)}
    entry = listing(0x591290, 0x5912A6)
    assert entry[0x591296] == ('mov', 'eax, dword ptr [0x9a64d4]')
    assert entry[0x59129E] == ('test', 'eax, eax')
    assert entry[0x5912A0] == ('je', '0x591852')
    tail = listing(0x591852, 0x591894)
    assert tail[0x591852] == ('mov', 'eax, dword ptr [0x68faf0]')
    assert tail[0x591857] == ('imul', 'eax, dword ptr [ebp + 0xc]')
    assert tail[0x59185B] == ('mov', 'ecx, dword ptr [ebp + 8]')
    assert tail[0x59185F] == ('sar', 'ecx, 1')
    assert tail[0x591863] == ('mov', 'ecx, dword ptr [0x94a30c]')
    assert tail[0x591874] == ('and', 'dl, 0x1f')
    assert tail[0x59187A] == ('mov', 'cl, byte ptr [ebp + 0x10]')
    assert tail[0x59187D] == ('shl', 'cl, 5')
    assert tail[0x591880] == ('or', 'dl, cl')
    assert tail[0x591882] == ('mov', 'byte ptr [eax], dl')
    assert tail[0x591884] == ('mov', 'eax, dword ptr [0x90d91c]')
    assert tail[0x591889] == ('or', 'al, 4')
    assert tail[0x59188B] == ('mov', 'dword ptr [0x90d91c], eax')
    assert tail[0x591893] == ('ret', '')
