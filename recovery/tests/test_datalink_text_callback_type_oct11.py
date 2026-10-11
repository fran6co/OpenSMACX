"""Pointer callback identity/ABI evidence, not caller or runtime completeness."""
import json
from pathlib import Path
import subprocess

from match_recovery.source import read_text
from match_recovery.source.writer import write_symbol
from match_recovery.tasks import Task

ROOT = Path(__file__).resolve().parents[2]
BASE = "99726732092cbfc451ed8cb8a5f04f459806c8e4"
SYMBOL = "?sub_44b340@@YAXP6AXPAD@Z@Z"
COMMENT = """// Native44B359 passes the caret string and44B36D reloads TextBufferGetPtr
// after that callback: these are text pointers, not integer callback values.
// No observed original caller connects this entry to the similar emit_help
// factoring below; keep that relationship unclaimed.
"""


def test_exact_native_writer_and_typed_body_migration():
    path = ROOT / "src/datalink.cpp"
    before = subprocess.check_output(["git", "show", BASE + ":src/datalink.cpp"])
    records = read_text(before.decode(), path)
    selected, = (r for r in records if r.address == 0x44B340)
    expected = write_symbol(before, selected, SYMBOL).decode()
    expected = expected.replace("void __cdecl sub_44b340(void (__cdecl *callback)(int))",
                                COMMENT + "void __cdecl sub_44b340(void (__cdecl *callback)(char *))")
    expected = expected.replace("callback(static_cast<int>(reinterpret_cast<long>(TechnologyHelpPrefixText)));",
                                "callback(TechnologyHelpPrefixText);")
    expected = expected.replace("callback(static_cast<int>(reinterpret_cast<long>(TextBufferGetPtr)));",
                                "callback(TextBufferGetPtr);")
    assert expected == path.read_text()
    from recovery.tests.test_texture_raster_actual_source import _continuity_fields
    assert [(_continuity_fields(r), r.function_fragment) for r in records] == [
        (_continuity_fields(r), r.function_fragment) for r in read_text(expected, path)]


def test_native_task_scope():
    task = Task.from_dict(json.loads((ROOT / "recovery/tasks/datalink-text-callback-type-oct11.json").read_text()))
    assert task.base_revision == BASE
    assert [s.address for s in task.subjects] == [0x44B340]
    assert set(task.editable_files) == {"src/datalink.cpp", "recovery/tests/test_datalink_techhelp_actual_source.py",
                                      "recovery/tests/test_datalink_text_callback_type_oct11.py",
                                      "recovery/tests/datalink_initializer_control.cpp",
                                      "recovery/tests/test_datalink_initializer.py"}


def test_pinned_pointer_arguments_and_post_callback_reload():
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32
    from match_recovery.pe_coff import PeImage
    from match_recovery.model import Span
    from match_recovery.source_inventory import image_pin
    image = PeImage((ROOT / ".opensmacx/game/terranx_original.exe").read_bytes(), image_pin(ROOT))
    instructions = {i.address: (i.mnemonic, i.op_str) for i in
                    Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x44B340, 0x44B37B)), 0x44B340)}
    assert instructions[0x44B359] == ("push", "0x684638")
    assert instructions[0x44B36D] == ("mov", "eax, dword ptr [0x9b7d00]")
    assert instructions[0x44B372] == ("push", "eax")
    assert instructions[0x44B373] == ("call", "esi")
    # The original completes the prefix callback before fetching the live
    # shared text pointer; do not substitute a pre-callback snapshot.
    calls = [a for a, (op, _) in instructions.items() if op == "call"]
    assert len(calls) == 2 and 0x44B359 < calls[0] < 0x44B36D < calls[1]
