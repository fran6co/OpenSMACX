"""Source identity/lifetime attachment controls, not native EH matching proof."""
from copy import deepcopy
from hashlib import sha256
import os
from pathlib import Path
import re
import struct

import pytest

from match_recovery import binding_source
from match_recovery.model import Span
from match_recovery.pe_coff import PeImage
from match_recovery.source import Mode, read, read_file, read_text


ROOT = Path(__file__).resolve().parents[2]
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def editor_record(text):
    rows = [row for row in read_text(text, ROOT / "src/console_commands.cpp")
            if row.address == 0x004DFAD0]
    assert len(rows) == 1
    return rows[0]


def assert_complete_editor_region(text):
    record = editor_record(text)
    assert record.mode is Mode.BODY
    assert record.image_spans == ((0x004DFAD0, 0x004E0118),
                                  (0x0065C0BE, 0x0065C0C8))
    signature = "void Console::editor_tech()"
    start = text.index(signature)
    opening = text.index("{", start)
    depth = 0
    for index in range(opening, len(text)):
        depth += (text[index] == "{") - (text[index] == "}")
        if depth == 0:
            definition = text[start:index + 1]
            break
    else:
        raise AssertionError("unterminated editor_tech definition")
    assert definition in record.region
    assert "Popup popup;" in definition and "Popup who;" in definition
    assert "TutWinBaseWindow.redraw();" in definition
    return record


def test_cleanup_identities_belong_to_real_owner_without_truncating_parent():
    text = (ROOT / "src/console_commands.cpp").read_text()
    parent = assert_complete_editor_region(text)
    rows = list(read(ROOT / "src"))
    for address, end, name in ((0x0065BFBC, 0x0065BFC7, "sub_65bfbc"),
                                (0x0065C001, 0x0065C009, "sub_65c001")):
        selected = [row for row in rows if row.address == address]
        assert len(selected) == 1
        child = selected[0]
        assert child.path == ROOT / "src/console_commands.cpp"
        assert child.line < parent.line
        assert child.image_spans == ((address, end),)
        assert child.name == name and child.kind == "game"
        assert child.mode is Mode.FILE and child.symbol == ""
        assert not child.byte_exact and not child.semantic
        assert not (ROOT / f"src/recovered/units/{address:08x}.cpp").exists()
        assert f'void {name}(' not in text


def test_nested_mapping_damage_cannot_hide_parent_body_truncation():
    text = (ROOT / "src/console_commands.cpp").read_text()
    assert_complete_editor_region(text)
    # Move an existing marker into the nested scope. This is disposable damage,
    # not a production writer or a new source identity/claim.
    marker = next(line for line in text.splitlines()
                  if line.startswith("// ORIGINAL: 0x0065BFBC "))
    damaged = text.replace(marker + "\n", "", 1).replace(
        "            Popup who;", marker + "\n            Popup who;", 1)
    with pytest.raises(AssertionError):
        assert_complete_editor_region(damaged)


def test_cleanup_member_path_has_real_automatic_cpp_owners():
    assert "class Popup : public BasePop" in (ROOT / "src/popup.h").read_text()
    assert "Scroll scroll_;" in (ROOT / "src/popup.h").read_text()
    assert "FlatButton flat_button_left_;" in (ROOT / "src/scroll.h").read_text()
    assert "class FlatButton : public BaseButton" in (ROOT / "src/flatbutton.h").read_text()


def test_popup_destructor_is_shared_inline_with_original_file_identity_retained():
    popup_h = (ROOT / "src/popup.h").read_text()
    popup_cpp = (ROOT / "src/popup.cpp").read_text()
    assert "~Popup() { close(); }" in popup_h
    assert "Popup::~Popup()" not in popup_cpp
    assert "void Popup::close()" in popup_cpp
    assert "scroll_.close();\n    BasePop::close();" in popup_cpp
    close_rows = [row for row in read(ROOT / "src") if row.address == 0x00404900]
    assert len(close_rows) == 1
    close, = close_rows
    assert close.path == ROOT / "src/popup.cpp"
    assert close.mode is Mode.BODY and close.byte_exact
    assert close.image_spans == ((0x00404900, 0x00404917),)
    rows = [row for row in read(ROOT / "src") if row.address == 0x00406C00]
    assert len(rows) == 1
    record, = rows
    assert record.path == ROOT / "src/popup.cpp"
    assert record.image_spans == ((0x00406C00, 0x00406E52),
                                  (0x00650AB0, 0x00650BA2))
    assert record.name == "??1Popup@@QAE@XZ"
    assert record.symbol == "??1Popup@@UAE@XZ"
    # Keep this as a FILE lifetime identity without forbidding later claim banking.
    assert record.mode is Mode.FILE
    assert '"reason": "Recover the full original operation using owned C++ members and base lifetimes; retain all original branches and expose unresolved callees instead of executing original-address forwarders."' in record.region


def test_all_original_unwind_actions_keep_spans_in_parent_lifetime_owner():
    text = (ROOT / "src/console_commands.cpp").read_text()
    parent = assert_complete_editor_region(text)
    # Independently bounded contiguous actions preceding the parent's handler.
    boundaries = (
        0x65BFB1, 0x65BFBC, 0x65BFC7, 0x65BFD2, 0x65BFDD, 0x65BFE5,
        0x65BFF3, 0x65C001, 0x65C009, 0x65C011, 0x65C01C, 0x65C027,
        0x65C032, 0x65C03D, 0x65C048, 0x65C053, 0x65C05E, 0x65C069,
        0x65C074, 0x65C07F, 0x65C08A, 0x65C092, 0x65C0A0, 0x65C0AE,
        0x65C0B6, 0x65C0BE,
    )
    records = list(read(ROOT / "src"))
    for start, end in zip(boundaries, boundaries[1:]):
        action, = [row for row in records if row.address == start]
        assert action.path == ROOT / "src/console_commands.cpp" and action.line < parent.line
        assert action.name == f"sub_{start:x}"
        assert action.image_spans == ((start, end),)
        assert action.mode is Mode.FILE and action.kind == "game"
        assert not action.symbol and not action.byte_exact and not action.semantic
        assert not (ROOT / f"src/unrecovered/{start:08x}.cpp").exists()
        assert f"void {action.name}(" not in text
    # This is source/lifetime navigation, not one-to-one current EH matching.
    assert parent.image_spans[-1] == (boundaries[-1], 0x65C0C8)


def _pinned_instructions(image, start, end):
    from capstone import Cs, CS_ARCH_X86, CS_MODE_32

    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(image.read(Span(start, end)), start))
    assert decoded and decoded[0].address == start
    assert sum(item.size for item in decoded) == end - start
    assert decoded[-1].address + decoded[-1].size == end
    return {item.address: item for item in decoded}


def _memory_operand(insn, index, base, displacement):
    from capstone.x86 import X86_OP_MEM

    operand = insn.operands[index]
    return (operand.type == X86_OP_MEM and operand.mem.base == base and
            operand.mem.index == 0 and operand.mem.disp == displacement)


def _register_operand(insn, index, register):
    from capstone.x86 import X86_OP_REG

    operand = insn.operands[index]
    return operand.type == X86_OP_REG and operand.reg == register


def _binding_producer_addresses(evidence):
    text = "\n".join(evidence["evidence"])
    return {int(value, 16) for value in re.findall(r"(?i)(?:0x)?([0-9a-f]{6,8})", text)}


def _assert_binding_owner(binding, owner_path, producer_address):
    assert binding["reason"].count(owner_path) == 1
    assert producer_address in _binding_producer_addresses(binding)


def _replace_address(text, old_address, new_address):
    def replace(match):
        old_token = match.group(2)
        if int(old_token, 16) != old_address:
            return match.group(0)
        new_token = f"{new_address:0{len(old_token)}X}"
        return (match.group(1) or "") + new_token

    return re.sub(r"(?i)(0x)?([0-9a-f]{6,8})", replace, text)


def test_right_scroll_button_cleanup_binding_uses_pinned_state_and_owner_path():
    from capstone.x86 import X86_OP_IMM, X86_OP_MEM
    from capstone.x86_const import X86_REG_EBP, X86_REG_ECX, X86_REG_EDX, X86_REG_ESI

    console_path = ROOT / "src/console_commands.cpp"
    source = console_path.read_text(encoding="utf-8")
    rows = read_file(console_path)
    action_rows = [row for row in rows if row.address == 0x0065C001]
    assert len(action_rows) == 1
    action, = action_rows
    assert action.image_spans == ((0x0065C001, 0x0065C009),)
    assert action.path == console_path
    bindings = binding_source.parse(source, rows)
    evidence = bindings[action.line]

    image_path = Path(os.environ.get(
        "OPENSMACX_IMAGE", str(ROOT / ".opensmacx/game/terranx_original.exe")))
    if not image_path.is_file():
        pytest.skip("pinned original executable unavailable")
    raw = image_path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)

    # The function's state-14 row is the 15th 8-byte unwind-map row. The
    # pinned map is nonmonotone in action-address order, so inspect this
    # bounded native row directly instead of applying the strict ordered-map API.
    row_address = 0x0067AB98 + 14 * 8
    assert row_address == 0x0067AC08
    predecessor, action_address = struct.unpack(
        "<II", image.read(Span(row_address, row_address + 8)))
    assert (predecessor, action_address) == (12, 0x0065C001)

    action_insns = _pinned_instructions(image, 0x0065C001, 0x0065C009)
    action_load = action_insns[0x0065C001]
    action_jump = action_insns[0x0065C004]
    assert action_load.mnemonic == "mov"
    assert _register_operand(action_load, 0, X86_REG_ECX)
    assert _memory_operand(action_load, 1, X86_REG_EBP, -0x14)
    assert action_jump.mnemonic == "jmp"
    assert action_jump.operands[0].type == X86_OP_IMM
    assert action_jump.operands[0].imm == 0x00607040

    # Pin producer: EBP-B70 enters the spill slot consumed above, distinct
    # from the later EBP-16BC left-button producer at 4DFEF1.
    producer = _pinned_instructions(image, 0x004DFEA8, 0x004DFEE5)
    lea_right = producer[0x004DFEA8]
    save_right = producer[0x004DFEB8]
    state_right = producer[0x004DFECD]
    close_right = producer[0x004DFED1]
    state_pop = producer[0x004DFEDC]
    dtor_right = producer[0x004DFEE0]
    assert lea_right.mnemonic == "lea"
    assert _register_operand(lea_right, 0, X86_REG_EDX)
    assert _memory_operand(lea_right, 1, X86_REG_EBP, -0xB70)
    assert save_right.mnemonic == "mov"
    assert _memory_operand(save_right, 0, X86_REG_EBP, -0x14)
    assert _register_operand(save_right, 1, X86_REG_EDX)
    assert state_right.mnemonic == "mov"
    assert _memory_operand(state_right, 0, X86_REG_EBP, -4)
    assert state_right.operands[1].imm == 14
    assert close_right.mnemonic == "call" and close_right.operands[0].imm == 0x00607DA0
    assert state_pop.mnemonic == "mov" and state_pop.operands[1].imm == 12
    assert dtor_right.mnemonic == "call" and dtor_right.operands[0].imm == 0x00607040

    # Pinned constructors establish the nested offset sum; source declarations
    # identify the real members occupying those offsets.
    popup_ctor = _pinned_instructions(image, 0x004048BF, 0x004048D6)
    popup_scroll = popup_ctor[0x004048C4]
    assert popup_scroll.mnemonic == "lea"
    assert _register_operand(popup_scroll, 0, X86_REG_ECX)
    assert popup_scroll.operands[1].type == X86_OP_MEM
    assert popup_scroll.operands[1].mem.base == X86_REG_ESI
    assert popup_scroll.operands[1].mem.disp == 0x3230

    scroll_ctor = _pinned_instructions(image, 0x006051F5, 0x00605214)
    right_button = scroll_ctor[0x00605204]
    assert right_button.mnemonic == "lea"
    assert _register_operand(right_button, 0, X86_REG_ECX)
    assert right_button.operands[1].type == X86_OP_MEM
    assert right_button.operands[1].mem.base == X86_REG_ESI
    assert right_button.operands[1].mem.disp == 0x15F8
    assert 0x5398 - 0x3230 - 0x15F8 == 0xB70

    popup_header = (ROOT / "src/popup.h").read_text(encoding="utf-8")
    scroll_header = (ROOT / "src/scroll.h").read_text(encoding="utf-8")
    flatbutton_header = (ROOT / "src/flatbutton.h").read_text(encoding="utf-8")
    assert "Scroll scroll_;" in popup_header
    assert "FlatButton flat_button_left_;" in scroll_header
    assert "FlatButton flat_button_right_;" in scroll_header
    assert "class FlatButton : public BaseButton" in flatbutton_header
    assert_complete_editor_region(source)

    right_owner = "scroll_.flat_button_right_"
    left_owner = "scroll_.flat_button_left_"
    _assert_binding_owner(evidence, right_owner, 0x004DFEB8)
    left_rows = [row for row in rows if row.address == 0x0065C009]
    assert len(left_rows) == 1
    left_action, = left_rows
    left_evidence = bindings[left_action.line]
    _assert_binding_owner(left_evidence, left_owner, 0x004DFEF1)

    # Both parts of the rationale are independently causal. A correct producer
    # cannot rescue the wrong member path, nor can the right path rescue a
    # binding whose evidence points at the left producer.
    wrong_owner = deepcopy(evidence)
    assert wrong_owner["reason"].count(right_owner) == 1
    wrong_owner["reason"] = wrong_owner["reason"].replace(right_owner, left_owner, 1)
    with pytest.raises(AssertionError):
        _assert_binding_owner(wrong_owner, right_owner, 0x004DFEB8)

    wrong_producer = deepcopy(evidence)
    wrong_producer["evidence"] = [
        _replace_address(row, 0x004DFEB8, 0x004DFEF1)
        for row in wrong_producer["evidence"]
    ]
    with pytest.raises(AssertionError):
        _assert_binding_owner(wrong_producer, right_owner, 0x004DFEB8)
