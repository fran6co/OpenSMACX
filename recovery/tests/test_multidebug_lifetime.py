"""Live MultiDebug callback/normal teardown ABI, not EH or GUI acceptance."""

from dataclasses import replace
import hashlib
import os
from pathlib import Path
import shutil
import struct

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_MEM

from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.codeview import ClassLayouts
from match_recovery.compiler import Compiler
from match_recovery.measurement import Measurement
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import Coff, PeImage
from match_recovery.source import read_file
from match_recovery.source_inventory import image_pin
from test_application_link_recovery import ROOT, run_recovered_harness


UNIT = "src/multidebug.cpp"
CALLBACK = "?timer_callback_daemon@MultiDebug@@SAXH@Z"
INIT = "?init@MultiDebug@@QAEHPAVNet@@PAVWin@@PAE@Z"
DTOR = "??1MultiDebug@@UAE@XZ"
TIME_START = "?start@Time@@QAEIP6AXH@ZHII@Z"
TIME_DTOR = "??1Time@@QAE@XZ"
FONT_DTOR = "??1Font@@QAE@XZ"
BASE_DTOR = "??1GraphicWin@@UAE@XZ"
PAINT = "?draw_contents@MultiDebug@@UAEXXZ"
GLOBAL = "?WinBubbleWindow@@3VMultiDebug@@A"
CTOR = "??0MultiDebug@@QAE@XZ"


@pytest.fixture(scope="module")
def family():
    from recovery.environment.wine import VC6_ROOT
    image_path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not (VC6_ROOT / "BIN/CL.EXE").is_file() or shutil.which("wine32") is None or not image_path.is_file():
        pytest.skip("VC6, Wine32 and the pinned image are required")
    settings = Measurement.read(ROOT)
    compiler = Compiler(ROOT, profile=settings.compiler_profile)
    objects = [Coff(compiler(ROOT / UNIT, (), flags + settings.layout_flags))
               for flags in settings.flag_sets]
    image = PeImage(image_path.read_bytes(), image_pin(ROOT))
    return settings, objects, image


def instructions(obj, name):
    code, relocs, _ = obj.subject(obj.function(name))
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoder.detail = True
    decoded = list(decoder.disasm(code, 0))
    assert sum(i.size for i in decoded) == len(code)
    return decoded, relocs


def referenced_instruction(obj, decoded, relocation):
    instruction, = (i for i in decoded
                    if i.address <= relocation.offset < i.address + i.size)
    return instruction, obj.symbols[int(relocation.target)].name


def assert_callback(obj):
    decoded, relocs = instructions(obj, CALLBACK)
    assert not relocs  # The redraw is virtual, not a direct base call.
    calls = [i for i in decoded if i.mnemonic in ("call", "jmp")]
    dispatch, = calls
    assert dispatch.op_str == "dword ptr [eax + 0xf8]"
    index = decoded.index(dispatch)
    assert (decoded[index - 1].mnemonic, decoded[index - 1].op_str) == (
        "mov", "eax, dword ptr [ecx]")
    guard, = (i for i in decoded if i.mnemonic == "je")
    guard_index = decoded.index(guard)
    assert (decoded[guard_index - 1].mnemonic, decoded[guard_index - 1].op_str) == (
        "test", "ecx, ecx")
    assert (decoded[guard_index - 2].mnemonic, decoded[guard_index - 2].op_str) in (
        ("mov", "ecx, dword ptr [ebp + 8]"),
        ("mov", "ecx, dword ptr [esp + 4]"))
    assert guard.address < dispatch.address < guard.operands[0].imm
    target, = (i for i in decoded if i.address == guard.operands[0].imm)
    assert target.mnemonic in ("pop", "ret")
    returns = [i for i in decoded if i.mnemonic == "ret"]
    assert len(returns) == 1 and not returns[0].op_str  # cdecl caller cleanup


def test_static_payload_callback_matches_original_and_virtual_null_guard(family):
    settings, objects, image = family
    record, = (r for r in read_file(ROOT / UNIT) if r.address == 0x5C9900)
    assert record.symbol == CALLBACK
    binding = FunctionBinding(
        Origin(hashlib.sha256(image.data).hexdigest(), "pe:va", record.address),
        tuple(Span(a, b) for a, b in record.image_spans), CALLBACK, UNIT)
    checked_framed = False
    for flags, obj in zip(settings.flag_sets, objects):
        assert_callback(obj)
        if "/Oy-" in flags:
            row, = check_objects([binding], {UNIT: obj}, image, external_targets={})
            assert row["boundary"] == row["bytes"] == row["references"] == "pass"
            checked_framed = True
    assert checked_framed


def assert_registration(obj):
    decoded, relocs = instructions(obj, INIT)
    refs = [referenced_instruction(obj, decoded, r) for r in relocs]
    starts = [i for i, name in refs if name == TIME_START]
    callbacks = [i for i, name in refs if name == CALLBACK]
    assert len(starts) == len(callbacks) == 1
    start, callback = starts[0], callbacks[0]
    assert start.mnemonic == "call" and callback.mnemonic == "push"
    index = decoded.index(start)
    # 5C98BC..5C98CF: resolution, period, original receiver payload,
    # real function address, then the Time member receiver and start call.
    setup = decoded[index - 5:index]
    assert [(i.mnemonic, i.op_str) for i in setup[:3]] == [
        ("push", "5"), ("push", "0x3e8"), ("push", "esi")]
    assert setup[3].address == callback.address
    assert (setup[4].mnemonic, setup[4].op_str) == ("lea", "ecx, [esi + 0xa40]")
    assert any(i.mnemonic == "mov" and i.op_str == "esi, ecx"
               for i in decoded[:index - 5])
    assert not any(i.mnemonic == "push" and i.op_str == "0x5c9900" for i in decoded)


def test_init_registers_real_callback_with_original_payload_and_timing(family):
    for obj in family[1]:
        assert_registration(obj)


def assert_normal_teardown(obj):
    decoded, relocs = instructions(obj, DTOR)
    calls = [(i, name) for i, name in
             (referenced_instruction(obj, decoded, r) for r in relocs)
             if i.mnemonic == "call"]
    assert [name for _, name in calls] == [TIME_DTOR, FONT_DTOR, BASE_DTOR]
    # A numeric original-address call can lack a relocation entirely. Do not
    # let the three ordinary member calls conceal an extra forwarder.
    assert [i.address for i in decoded if i.mnemonic == "call"] == [
        i.address for i, _ in calls]
    clears = [i for i in decoded
              if i.mnemonic == "mov" and i.op_str == "dword ptr [esi + 0xa3c], 0"]
    assert len(clears) == 1
    clear = clears[0]
    assert clear.address < calls[0][0].address
    # Assert the member receivers as well as the destructor names; metadata
    # labels and exception-handler references are deliberately not frozen.
    for (call, _), setup in zip(calls, [
        ("lea", "ecx, [esi + 0xa40]"),
        ("lea", "ecx, [esi + 0xa14]"), ("mov", "ecx, esi")]):
        prior = decoded[:decoded.index(call)]
        receiver = next(i for i in reversed(prior)
                        if i.mnemonic in ("mov", "lea") and i.op_str.startswith("ecx, "))
        assert (receiver.mnemonic, receiver.op_str) == setup
    returns = [i for i in decoded if i.mnemonic == "ret"]
    assert len(returns) == 1 and not returns[0].op_str


def test_destructor_clears_session_then_destroys_each_owned_subobject_once(family):
    for obj in family[1]:
        assert_normal_teardown(obj)


def change_displacement(obj, name, instruction, value):
    assert any(o.type == X86_OP_MEM for o in instruction.operands)
    assert instruction.disp_size == 4
    symbol = obj.function(name)
    section = obj.sections[symbol.section - 1]
    code = bytearray(section.data)
    struct.pack_into("<i", code, symbol.value + instruction.address + instruction.disp_offset, value)
    obj.sections = tuple(replace(s, data=bytes(code)) if n == symbol.section - 1 else s
                         for n, s in enumerate(obj.sections))


@pytest.mark.parametrize("damage", ["wrong_slot", "missing_clear", "wrong_order", "wrong_registration"])
def test_compiled_controls_reject_callback_and_ownership_damage(family, damage):
    for original in family[1]:
        obj = Coff(original.raw_data)
        if damage == "wrong_slot":
            decoded, _ = instructions(obj, CALLBACK)
            dispatch, = (i for i in decoded if i.mnemonic in ("call", "jmp"))
            change_displacement(obj, CALLBACK, dispatch, 0xfc)
            check = assert_callback
        elif damage == "missing_clear":
            decoded, _ = instructions(obj, DTOR)
            clear, = (i for i in decoded if i.op_str == "dword ptr [esi + 0xa3c], 0")
            change_displacement(obj, DTOR, clear, 0xa38)
            check = assert_normal_teardown
        else:
            name = DTOR if damage == "wrong_order" else INIT
            symbol = obj.function(name)
            section = obj.sections[symbol.section - 1]
            targets = [TIME_DTOR, FONT_DTOR] if damage == "wrong_order" else [CALLBACK, TIME_START]
            indices = {target: next(index for _, index, _ in section.relocations
                                   if obj.symbols[index].name == target) for target in targets}
            swaps = {indices[targets[0]]: indices[targets[1]]}
            if damage == "wrong_order":
                swaps[indices[targets[1]]] = indices[targets[0]]
            changed = tuple((offset, swaps.get(index, index), kind)
                            for offset, index, kind in section.relocations)
            obj.sections = tuple(replace(s, relocations=changed) if n == symbol.section - 1 else s
                                 for n, s in enumerate(obj.sections))
            check = assert_normal_teardown if damage == "wrong_order" else assert_registration
        with pytest.raises(AssertionError):
            check(obj)


def test_real_paint_slot_member_layout_and_global_owner(family):
    _, objects, image = family
    # Independent original primary-table slot, not inferred from our declaration.
    assert struct.unpack("<I", image.read(Span(0x66FAB8, 0x66FABC)))[0] == 0x5C9920
    for obj in objects:
        table, = (s for s in obj.symbols.values()
                  if s.name == "??_7MultiDebug@@6BWin@@@" and s.section > 0)
        targets = {offset - table.value: obj.symbols[index].name
                   for offset, index, _ in obj.sections[table.section - 1].relocations}
        assert targets[12 * 4] == PAINT
        assert targets[62 * 4] == "?redraw@GraphicWin@@UAEXXZ"
        obj.function(PAINT)
        layouts = ClassLayouts(obj)
        debug = layouts.layout("MultiDebug")
        assert [(b["class"], b["offset"], b["virtual"]) for b in debug["bases"]] == [
            ("GraphicWin", 0, False)]
        assert {n: debug["members"][n] for n in ("font_", "net_", "timer_")} == {
            "font_": 0xA14, "net_": 0xA3C, "timer_": 0xA40}
        sound = layouts.layout("AutoSound")
        assert sound["members"]["val_4_"] == 0x10
        assert sound["member_types"]["val_4_"] == {
            "kind": "pointer", "width": 4, "target": {"kind": "class_value", "class": "Win"}}
        player = layouts.layout("NetPlayer")
        assert player["size"] == 0x58
        assert player["members"]["flags_14_"] == 0x14
        assert player["members"]["name_and_tail_"] == 0x15
        tail = player["member_types"]["name_and_tail_"]
        assert tail["kind"] == "array" and tail["size"] == 0x43
        # Storage to the known slot boundary is NOT a recovered name capacity.
        owner, = (s for s in obj.symbols.values()
                  if s.name == "?WinBubbleWindow@@3VMultiDebug@@A" and s.section > 0)
        assert owner.storage == 2
        assert not any(s.name == "?WinBubbleWindow@@3VWin@@A" for s in obj.symbols.values())
    # The owning definition is above; do not compile another full Win TU just
    # to check that the retired aliases have disappeared from its source.
    win = (ROOT / "src/win.cpp").read_text()
    for retired in ("WinDrawFlags", "WinPopupWindow", "WinDialogList", "WinPopupCount", "WinTopDialog"):
        assert retired not in win


def test_live_paint_formats_all_slots_and_rereads_callback_state(tmp_path, monkeypatch):
    # FILE mapping owns compiler output; only its uniquely delimited live body
    # belongs in this fixture, never the complete translation unit or a copy.
    run_recovered_harness(tmp_path, monkeypatch, "multidebug_paint_harness.cpp", [], declarations=[
        ("// LIVE_PLAYER", "src/net_class.h", "class NetPlayer {", "\n\nstruct ServiceList;"),
        ("// LIVE_PAINT", UNIT, "void MultiDebug::draw_contents() {",
         "\n/*\nPurpose: Step the receiver back to the subobject ??_GMultiDebug@@")])


@pytest.mark.parametrize("damage", ["last_slot", "signed_flags", "one_space", "cached_session"])
def test_live_paint_controls_reject_formatting_and_stale_session(tmp_path, monkeypatch, damage):
    original_read = Path.read_text

    def damaged_read(path, *args, **kwargs):
        text = original_read(path, *args, **kwargs)
        if path == ROOT / UNIT:
            start = "void MultiDebug::draw_contents() {"
            end = "\n/*\nPurpose: Step the receiver back to the subobject ??_GMultiDebug@@"
            assert text.count(start) == text.count(end) == 1
            first, last = text.index(start), text.index(end)
            body = text[first:last]
            if damage == "cached_session":
                assert body.count("net_->") >= 8
                body = body.replace("net_->", "session->").replace(
                    start, start + "\n    Net *session = net_;")
            else:
                good, bad = {
                    "last_slot": ("index < 16", "index < 15"),
                    "signed_flags": ("_itoa(net_->players_[index].flags_14_, flags, 10);",
                                     "_itoa(static_cast<signed char>(net_->players_[index].flags_14_), flags, 10);"),
                    "one_space": ('strcat(message, " ");', 'strcat(message, "");'),
                }[damage]
                assert body.count(good) == 1
                body = body.replace(good, bad)
            text = text[:first] + body + text[last:]
        return text

    monkeypatch.setattr(Path, "read_text", damaged_read)
    with pytest.raises(AssertionError, match="Assertion failed"):
        test_live_paint_formats_all_slots_and_rereads_callback_state(tmp_path, monkeypatch)


def local_function(obj, name):
    matches = [s for s in obj.symbols.values()
               if s.name == name and s.section > 0 and s.storage == 3 and s.type & 0x20]
    return len(matches) == 1


def helper_instructions(obj, name):
    symbol = obj.function(name, comdat=False)
    code, relocs, _ = obj.subject(symbol)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)
    decoded = list(decoder.disasm(code, 0))
    assert sum(i.size for i in decoded) == len(code)
    return decoded, relocs


def helper_events(obj, name):
    decoded, relocs = helper_instructions(obj, name)
    events = [(name, *referenced_instruction(obj, decoded, r), r) for r in relocs]
    return decoded, events


def assert_static_lifetime_chain(obj):
    # Discover the relevant CRT root through its relocation graph. Other roots
    # (e.g. the C++ library locale initializer) are not our lifetime or waiver.
    roots = [obj.symbols[index].name for section in obj.sections
             if section.name == ".CRT$XCU" for _, index, kind in section.relocations
             if kind == 6]

    def reachable(name, seen=()):
        assert name not in seen and len(seen) < 8
        _, events = helper_events(obj, name)
        names = {target for _, _, target, _ in events}
        for _, instruction, target, _ in events:
            if instruction.mnemonic in ("call", "jmp") and local_function(obj, target):
                names.update(reachable(target, (*seen, name)))
        return names

    candidates = [root for root in roots if {GLOBAL, CTOR, DTOR} & reachable(root)]
    assert len(candidates) == 1
    root = candidates[0]

    def flatten(name, seen=()):
        assert name not in seen and len(seen) < 8
        decoded, events = helper_events(obj, name)
        assert not any(i.mnemonic.startswith("j") and i.mnemonic != "jmp" for i in decoded)
        assert [i.address for i in decoded if i.mnemonic in ("call", "jmp")] == [
            i.address for _, i, _, _ in events if i.mnemonic in ("call", "jmp")]
        result = []
        for event in events:
            _, instruction, target, _ = event
            if instruction.mnemonic in ("call", "jmp") and local_function(obj, target):
                result.extend(flatten(target, (*seen, name)))
            else:
                result.append(event)
        return result

    # Both the split helper chain and a fused initializer must reduce to the
    # same observable order. No compiler-generated _$E number is prescribed.
    events = flatten(root)
    assert len(events) == 4
    assert [event[2] for event in events[:2]] == [GLOBAL, CTOR]
    assert (events[0][1].mnemonic, events[0][1].op_str) == ("mov", "ecx, 0")
    assert events[1][1].mnemonic in ("call", "jmp")
    assert events[2][1].mnemonic == "push"
    registration_owner, registration, target, _ = events[3]
    assert target == "_atexit" and registration.mnemonic == "call"
    assert events[2][0] == registration_owner
    decoded, _ = helper_instructions(obj, registration_owner)
    registration_index, = (index for index, instruction in enumerate(decoded)
                            if instruction.address == registration.address)
    following = decoded[registration_index + 1:]
    assert following
    # Caller removes the one pushed callback argument. VC6 also uses pop ecx.
    assert (following[0].mnemonic, following[0].op_str) in (
        ("add", "esp, 4"), ("pop", "ecx"))
    assert any(i.mnemonic == "ret" and not i.op_str for i in following)

    teardown_name = events[2][2]
    teardown_code, teardown = helper_events(obj, teardown_name)
    assert len(teardown) == 2
    assert [event[2] for event in teardown] == [GLOBAL, DTOR]
    assert (teardown[0][1].mnemonic, teardown[0][1].op_str) == ("mov", "ecx, 0")
    assert teardown[1][1].mnemonic == "jmp"
    assert [i.address for i in teardown_code if i.mnemonic in ("call", "jmp")] == [
        teardown[1][1].address]
    return root, teardown_name, events


def test_crt_constructs_owned_debug_window_and_registers_its_complete_teardown(family):
    records = {r.address: r for r in read_file(ROOT / UNIT)}
    for obj in family[1]:
        root, teardown, _ = assert_static_lifetime_chain(obj)
        assert records[0x5C97C0].symbol == root
        assert records[0x5C97E0].symbol == teardown


@pytest.mark.parametrize("damage", ["wrong_ctor", "wrong_object", "wrong_teardown"])
def test_crt_lifetime_controls_reject_wrong_owner_or_registered_operation(family, damage):
    for original in family[1]:
        obj = Coff(original.raw_data)
        _, _, events = assert_static_lifetime_chain(obj)
        event = events[{"wrong_object": 0, "wrong_ctor": 1, "wrong_teardown": 2}[damage]]
        owner, _, _, relocation = event
        replacement_name = {"wrong_object": "??_7MultiDebug@@6BWin@@@",
                            "wrong_ctor": DTOR, "wrong_teardown": CALLBACK}[damage]
        replacements = [s for s in obj.symbols.values()
                        if s.name == replacement_name and s.section > 0]
        assert len(replacements) == 1
        section_number = obj.function(owner, comdat=False).section
        section = obj.sections[section_number - 1]
        changed = tuple((offset, replacements[0].index if offset == relocation.offset else index, kind)
                        for offset, index, kind in section.relocations)
        obj.sections = tuple(replace(s, relocations=changed) if n == section_number - 1 else s
                             for n, s in enumerate(obj.sections))
        with pytest.raises(AssertionError):
            assert_static_lifetime_chain(obj)
