"""FX producer call/data contract, with explicit audio doubles, not game acceptance."""

from collections import Counter
import json
import os
from pathlib import Path
import shutil

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_OP_REG

from match_recovery.compiler import Compiler
from match_recovery.codeview import class_layout
from match_recovery.measurement import Measurement
from match_recovery.model import Span
from match_recovery.object_data import check_initial_data
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, references, run_recovered_harness


UNIT = "src/fx.cpp"
SYMBOL = "?init@FX@@QAEXXZ"
THIS = 0x10000000
GLOBAL_WAVES = (0x74D8F0, 0x6A7090, 0x749C18, 0x74D9D0, 0x749C88, 0x74D960)


@pytest.fixture(scope="module")
def original():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original image is required")
    return PeImage(path.read_bytes(), image_pin(ROOT))


def original_trace(image):
    """Execute this bounded initializer's integer/control subset, not its callees.

    Calls obey their independently identified thiscall argument counts, preserve
    EBX/ESI/EDI, and clobber volatile registers. The observed Effect vptr supplies
    the sole indirect target. Refuse other instructions/reads, not silent skips.
    """
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    ops = {op.address: op for op in decoder.disasm(image.read(Span(0x445CD0, 0x4468E1)), 0x445CD0)}
    values = {"ecx": THIS, "ebx": -1, "esi": -2, "edi": -3}
    stack, memory, calls = [], {}, []
    signatures = {0x445BB0: ("effect_init", 2), 0x4C6DB0: ("load", 2),
                  0x4C7130: ("volume", 1), 0x4C5240: ("group", 2),
                  0x4C6EE0: ("pitch", 1), 0x4C69B0: ("wave_init", 2),
                  0x4C66B0: ("pan", 1)}

    def address(op, operand):
        mem = operand.mem
        return ((values[op.reg_name(mem.base)] if mem.base else 0) +
                (values[op.reg_name(mem.index)] * mem.scale if mem.index else 0) + mem.disp)

    def read(op, operand):
        if operand.type == X86_OP_REG:
            return values[op.reg_name(operand.reg)]
        if operand.type == X86_OP_IMM:
            return operand.imm & 0xFFFFFFFF
        assert operand.type == X86_OP_MEM and operand.size == 4
        location = address(op, operand)
        if location == THIS + 4 * 108:
            return 0x66B7A4  # Effect construction's independently observed table.
        assert location == 0x66B7A4 + 38 * 4, hex(location)
        return int.from_bytes(image.read(Span(location, location + 4)), "little")

    def owner(value):
        if THIS <= value < THIS + 97 * 108:
            assert (value - THIS) % 108 == 0
            return 1, (value - THIS) // 108
        if 0x74C5F0 <= value < 0x74D8EC:
            assert (value - 0x74C5F0) % 108 == 0
            return 2, (value - 0x74C5F0) // 108
        return 3, GLOBAL_WAVES.index(value)

    pc, comparison = 0x445CD0, None
    for _ in range(3000):
        op = ops[pc]
        pc += op.size
        operands = op.operands
        if op.mnemonic == "push":
            stack.append(read(op, operands[0]))
        elif op.mnemonic == "pop":
            values[op.reg_name(operands[0].reg)] = stack.pop()
        elif op.mnemonic in ("mov", "lea"):
            value = address(op, operands[1]) if op.mnemonic == "lea" else read(op, operands[1])
            if operands[0].type == X86_OP_REG:
                values[op.reg_name(operands[0].reg)] = value
            else:
                assert op.mnemonic == "mov" and operands[0].size == 4
                memory[address(op, operands[0])] = value
        elif op.mnemonic == "xor":
            assert operands[0].reg == operands[1].reg
            values[op.reg_name(operands[0].reg)] = 0
        elif op.mnemonic == "add":
            values[op.reg_name(operands[0].reg)] += read(op, operands[1])
        elif op.mnemonic == "test":
            comparison = read(op, operands[0]) & read(op, operands[1])
        elif op.mnemonic == "cmp":
            comparison = read(op, operands[0]) - read(op, operands[1])
        elif op.mnemonic in ("je", "jl"):
            assert comparison is not None
            if (comparison == 0 if op.mnemonic == "je" else comparison < 0):
                pc = read(op, operands[0])
        elif op.mnemonic == "rep movsd":
            assert (values["esi"], values["edi"], values["ecx"]) == (0x683AD8, 0x9B8AB8, 70)
            for i in range(values["ecx"]):
                start = values["esi"] + i * 4
                memory[values["edi"] + i * 4] = int.from_bytes(image.read(Span(start, start + 4)), "little")
            values["esi"] += 280
            values["edi"] += 280
            values["ecx"] = 0
        elif op.mnemonic == "call":
            target = read(op, operands[0])
            name, count = signatures[target]
            args = [stack.pop() for _ in range(count)]
            receiver = values["ecx"]
            indirect = operands[0].type != X86_OP_IMM
            if indirect:
                assert target == 0x4C6EE0 and receiver == THIS + 4 * 108
                name = "virtual_pitch"
            text = ""
            if name in ("load", "effect_init", "wave_init"):
                text = image.read(Span(args[0], args[0] + 128)).split(b"\0", 1)[0].decode("ascii")
                value = args[1]
            elif name == "group":
                assert receiver == 0x90D978
                receiver, value = args[1], args[0]
            else:
                value = args[0]
                if value >= 0x80000000:
                    value -= 0x100000000
            # Defaults must already be published before the very first callee.
            assert memory[THIS + 0x28EC] == 0xFFFFFFFF and memory[0x9B8CF8] == 0x4456B0
            assert len([a for a in memory if 0x9B8AB8 <= a < 0x9B8BD0]) == 70
            reset = {a: v for a, v in memory.items() if 0x9BC080 <= a < 0x9BC114}
            assert not reset or reset == {a: 0 for a in range(0x9BC080, 0x9BC114, 4)}
            calls.append((name, *owner(receiver), text, value, int(bool(reset))))
            for register in ("eax", "ecx", "edx"):
                values.pop(register, None)
        elif op.mnemonic == "ret":
            assert pc == 0x4468E1 and not stack
            assert {a for a in memory if a not in (THIS + 0x28EC, 0x9B8CF8)} == (
                set(range(0x9B8AB8, 0x9B8BD0, 4)) | set(range(0x9BC080, 0x9BC114, 4)))
            return calls
        else:
            raise AssertionError((hex(op.address), op.mnemonic, op.op_str))
    raise AssertionError("original trace did not terminate")


def test_original_initialization_trace_has_complete_receiver_and_argument_coverage(original):
    trace = original_trace(original)
    assert len(trace) == 199
    counts = Counter(row[0] for row in trace)
    assert counts["effect_init"] == 45 and counts["wave_init"] == 43
    assert [row[2] for row in trace if row[0] == "wave_init"] == [
        *range(19), *range(20, 31), *range(32, 41), 42, 41, 43, 44]
    assert sum(row[0] == "group" and row[1] == 2 for row in trace) == 45
    assert [row for row in trace if row[0] == "virtual_pitch"] == [("virtual_pitch", 1, 4, "", 1100, 0)]


def test_actual_initializer_follows_original_call_and_publication_sequence(original, tmp_path, monkeypatch):
    expected = original_trace(original)
    read_text = Path.read_text

    def supplied(path, *args, **kwargs):
        text = read_text(path, *args, **kwargs)
        if path.name == "fx_initialization_harness.cpp":
            rows = ["    {%s, %d, %d, %s, %d, %d}," %
                    (json.dumps(op), family, index, json.dumps(name), value, reset)
                    for op, family, index, name, value, reset in expected]
            text = text.replace("// ORIGINAL_CALL_CONTRACT", "\n".join(rows))
        return text

    monkeypatch.setattr(Path, "read_text", supplied)
    run_recovered_harness(tmp_path, monkeypatch, "fx_initialization_harness.cpp", [
        ("// LIVE_INITIALIZER", UNIT, 0x445CD0)], declarations=[
        ("// LIVE_DEFAULTS_DECLARATION", "src/autosound_defaults.h", "struct AutoSoundDefaultsBlock {", "\n\nextern AutoSoundDefaultsBlock"),
        ("// LIVE_TEXT_BLOCK", "src/basepop.h", "struct BasePopTextBlock {", "static_assert(sizeof(BasePopTextBlock)"),
        ("// LIVE_FX_DECLARATION", "src/fx.h", "class FX {", "static_assert(sizeof(FX)"),
        ("// LIVE_DEFAULT_VALUE", UNIT, "const BasePopTextBlock FxPopupSoundDefaults", "// Initialize the effect")])


@pytest.mark.parametrize("before,after", [
    ("effects_[96].init", "effects_[95].init"),
    ("effects_[4].set_pitch(1100)", "effects_[4].Wave::set_pitch(1100)"),
    ("index < 45", "index < 44"),
    ('"wpn laser2.wav", 0xC0', '"wpn laser2.wav", 0x80'),
    ("sizeof(AutoSoundDefaults)", "sizeof(AutoSoundDefaults) - 4"),
    ("sizeof(FxPopupSoundDefaults)", "sizeof(FxPopupSoundDefaults) - 4"),
    ("BasePopDefaultField3094 = popup_wave_callback;", "BasePopDefaultField3094 = 0;"),
])
def test_initializer_contract_rejects_damage(original, tmp_path, monkeypatch, before, after):
    read_text = Path.read_text

    def damaged(path, *args, **kwargs):
        text = read_text(path, *args, **kwargs)
        if path == ROOT / UNIT:
            assert text.count(before) == 1
            text = text.replace(before, after)
        return text

    monkeypatch.setattr(Path, "read_text", damaged)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_actual_initializer_follows_original_call_and_publication_sequence(original, tmp_path, monkeypatch)


def test_compiled_initializer_uses_real_owners_and_one_virtual_pitch_call(original):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    binding, = (b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol == "_FxPopupSoundDefaults")
    defaults, = (b for b in read_objects(ROOT, image_pin(ROOT))
                 if b.symbol == "?AutoSoundDefaults@@3PAIA")
    assert binding.span == Span(0x683AD8, 0x683BF0) and binding.evidence.address == 0x445CD0
    for flags in settings.flag_sets:
        obj = Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
        row, = check_initial_data([binding], {UNIT: obj}, original, {})
        assert row["content"] == row["proof"]["boundary"] == "pass", row
        constant, = (s for s in obj.symbols.values()
                     if s.name == binding.symbol and s.section > 0)
        section = obj.sections[constant.section - 1]
        assert class_layout(obj, "BasePopTextBlock")["size"] == 280
        assert section.data[constant.value:constant.value + 280] == original.read(binding.span)
        targets = references(obj, SYMBOL)
        assert {"?g_CPU_WAVES@@3PAVWave@@A", "?g_OK_WAVE@@3VWave@@A",
                "?WaveDeviceGlobal@@3VWave_Device@@A", defaults.emitted_symbol,
                "?BasePopDefaultBlock2E64@@3PAIA", "?popup_wave_callback@@YAXPAUBasePopTextBlock@@H@Z"} <= set(targets)
        assert not any("pending" in target.lower() or "Original" in target or "vtable_slot" in target for target in targets)
        code, _, _ = obj.subject(obj.function(SYMBOL))
        decoder = Cs(CS_ARCH_X86, CS_MODE_32)
        decoder.detail = True
        indirect = [op for op in decoder.disasm(code, 0) if op.mnemonic == "call" and op.operands[0].type != X86_OP_IMM]
        assert len(indirect) == 1 and indirect[0].operands[0].type == X86_OP_MEM
        assert indirect[0].operands[0].mem.disp == 38 * 4


@pytest.mark.parametrize("replacement,dimension", [
    ('const BasePopTextBlock FxPopupSoundDefaults = {"", -1, 126, 50, 0, 0};', "bytes"),
    ('const BasePopTextBlock FxPopupSoundDefaults[2] = {{"", -1, 127, 50, 0, 0}};', "boundary"),
])
def test_native_static_default_check_rejects_value_and_extent_damage(original, replacement, dimension):
    from recovery.environment.wine import VC6_ROOT
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None:
        pytest.skip("VC6 and Wine32 are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    binding, = (b for b in read_objects(ROOT, image_pin(ROOT)) if b.symbol == "_FxPopupSoundDefaults")
    path = ROOT / UNIT
    source = path.read_bytes()
    declaration = b'const BasePopTextBlock FxPopupSoundDefaults = {"", -1, 127, 50, 0, 0};'
    assert source.count(declaration) == 1
    for flags in settings.flag_sets:
        pristine = Coff(compiler(path, (), flags + settings.layout_flags))
        control, = check_initial_data([binding], {UNIT: pristine}, original, {})
        assert control["content"] == "pass", control
        damaged = Coff(compiler.overlay(path, {UNIT: source.replace(declaration, replacement.encode())},
                                        (), flags + settings.layout_flags))
        row, = check_initial_data([binding], {UNIT: damaged}, original, {})
        assert row["definition"] == "pass" and row["content"] == row["proof"][dimension] == "fail", row
    assert path.read_bytes() == source
