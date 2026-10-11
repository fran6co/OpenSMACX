"""Pinned table-index evidence for reachable high raw-rule transfers."""
from hashlib import sha256
import os
from pathlib import Path
import struct

import pytest

ROOT = Path(os.environ.get("OPENSMACX_ROOT", Path(__file__).resolve().parents[2]))
PIN = "01901cbf7196b0c5d0df9540a029520f5df8fd9a6b343deef8b5663872805fcf"


def test_high_rule_dispatch_is_indexed_from_the_guarded_native_tables():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def decode(start, end):
        return {i.address: (i.mnemonic, i.op_str.lower())
                for i in decoder.disasm(image.read(Span(start, end)), start)}

    size_targets = (0x5331EC, 0x533226, 0x533230, 0x53323A, 0x533273,
                    0x53327D, 0x533287, 0x532FF3, 0x5332C0, 0x5332CA,
                    0x5332D1, 0x5332D8, 0x53330E, 0x533315)
    address_targets = (0x53380A, 0x533A5C, 0x533A63, 0x533820, 0x533A6A,
                       0x533836, 0x53384C, 0x533862, 0x533874, 0x533886,
                       0x533898, 0x5338AA, 0x5338BC, 0x5338CE)
    commit_targets = (0x533CAB, 0x533F66, 0x533F66, 0x533CF0, 0x533F66,
                      0x533D2E, 0x533D60, 0x533DAB, 0x533DCD, 0x533E07,
                      0x533E3B, 0x533E70, 0x533EAE, 0x533EE7)
    # These are physical indexed table reads, not inferred adjacent labels.
    for base, expected in ((0x5341D4, size_targets), (0x5342B4, address_targets),
                           (0x5342EC, commit_targets)):
        assert struct.unpack("<14I", image.read(Span(base, base + 14 * 4))) == expected
    guards = {}
    for start, end in ((0x532E45, 0x532E55), (0x533444, 0x533454),
                       (0x5331D9, 0x5331EC), (0x5337F7, 0x53380A),
                       (0x533C81, 0x533CAB)):
        guards.update(decode(start, end))
    for address, instruction in {
        0x532E45: ("cmp", "eax, 0x29"), 0x532E48: ("ja", "0x533349"),
        0x533444: ("cmp", "eax, 0x29"), 0x533447: ("ja", "0x533a77"),
        0x5331D9: ("add", "eax, -0x80"), 0x5331DC: ("cmp", "eax, 0xd"),
        0x5331E5: ("jmp", "dword ptr [eax*4 + 0x5341d4]"),
        0x5337F7: ("add", "eax, -0x80"), 0x5337FA: ("cmp", "eax, 0xd"),
        0x533803: ("jmp", "dword ptr [eax*4 + 0x5342b4]"),
        0x533C88: ("movsx", "edx, word ptr [ecx + 0x12]"),
        0x533C8E: ("jne", "0x533f66"),
        0x533C98: ("add", "eax, -0x80"), 0x533C9B: ("cmp", "eax, 0xd"),
        0x533CA4: ("jmp", "dword ptr [eax*4 + 0x5342ec]"),
    }.items():
        assert guards[address] == instruction

    bases = {0x80: 0x94F358, 0x81: 0x949730, 0x82: 0x9502A8,
             0x83: 0x691878, 0x84: 0x945F50, 0x85: 0x94F1B8,
             0x86: 0x94A330, 0x87: 0x9527F8, 0x88: 0x94AE60,
             0x89: 0x94F278, 0x8A: 0x9AB538, 0x8B: 0x9A4B68,
             0x8C: 0x946020, 0x8D: 0x94B000}
    for opcode, base in bases.items():
        target = address_targets[opcode - 0x80]
        if opcode in (0x81, 0x82, 0x84):
            route = decode(target, target + 7)
            assert route == {target: ("mov", f"esi, {base:#x}"),
                             target + 5: ("jmp", "0x533a77")}
            assert commit_targets[opcode - 0x80] == 0x533F66
        else:
            route = tuple(decode(target, target + 22).values())[:5]
            assert route == (("mov", "eax, dword ptr [ebp + 0x1c]"),
                             ("test", "eax, eax"), ("je", "0x5338e0"),
                             ("add", f"esi, {base:#x}"), ("jmp", "0x533a77"))
    assert decode(0x5338E0, 0x5338E6)[0x5338E0] == ("add", "esi, 0x93ad8c")
    assert decode(0x533A77, 0x533A82)[0x533A7C] == ("je", "0x5338e6")

    sizes = (0xF4C, 0x13C, 0x7C, 0x280, 0x90, 0xC0, 0x510,
             0x30, 0x1A0, 0xE0, 0x32C, 0x1920, 0x118, 0x350)
    for target, size in zip(size_targets, sizes):
        route = decode(target, target + 0x40)
        assert ("mov", f"esi, {size:#x}") in route.values()
    for start, compare, calls, store, phase in (
        (0x5331EC, 0x5331F8, (0x533208, 0x53320D), 0x533212, 1),
        (0x53323A, 0x533245, (0x533255, 0x53325A), 0x53325F, 2),
        (0x533287, 0x533292, (0x5332A2, 0x5332A7), 0x5332AC, 3),
        (0x5332D8, 0x5332E3, (0x5332F3, 0x5332F8), 0x5332FD, 4),
        (0x533315, 0x533320, (0x533327, 0x53332C), 0x533331, 5),
    ):
        route = decode(start, store + 10)
        assert route[compare][0] == ("jl" if phase == 1 else "jne")
        assert all(route[address][0] == "call" for address in calls)
        assert route[store] == ("mov", f"dword ptr [edi + 0x1b4c], {phase}")
    assert decode(0x533315, 0x53333B)[0x533319] == ("cmp", "dword ptr [edi + 0x1b4c], 4")


def test_counter_rules_window_preserves_three_native_object_identities():
    from match_recovery.object_source import parse_objects

    source = (ROOT / "src/alpha.cpp").read_text()
    records = parse_objects(source, "src/alpha.cpp", PIN, address_space="va")
    expected = {0x949730: ("?TechValidCount@@3HA", "tech_valid_count_"),
                0x949734: ("?TechCommerceCount@@3HA", "tech_commerce_count_"),
                0x949738: ("?Rules@@3URulesBasic@@A", "rules_")}
    selected = {row.origin.address: row for row in records if row.origin.address in expected}
    assert set(selected) == set(expected)
    for address, (symbol, member) in expected.items():
        row = selected[address]
        assert (row.symbol, row.parent_symbol, row.member_path) == (
            symbol, "?RulesStorage@@3URulesStorageWindow@@A", (member,))
        assert row.span is None  # No invented complete allocation extent.
    technology = (ROOT / "src/technology.cpp").read_text()
    assert "int TechValidCount;" not in technology
    assert "int TechCommerceCount;" not in technology


def test_supported_variable_sizes_keep_native_signed_chunk_flow():
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    from match_recovery.model import Span
    from match_recovery.pe_coff import PeImage

    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original unavailable")
    raw = path.read_bytes()
    assert sha256(raw).hexdigest() == PIN
    image = PeImage(raw, PIN)
    decoder = Cs(CS_ARCH_X86, CS_MODE_32)

    def decode(start, end):
        return {i.address: (i.mnemonic, i.op_str.lower())
                for i in decoder.disasm(image.read(Span(start, end)), start)}

    # Source scalar inputs correspond to these indexed size-table arms.
    targets = {0x0F: 0x532F85, 0x12: 0x532FFD, 0x14: 0x5330A1,
               0x20: 0x533117, 0x22: 0x533166}
    for opcode, target in targets.items():
        assert struct.unpack("<I", image.read(Span(0x53412C + opcode * 4,
                                                  0x534130 + opcode * 4))) == (target,)
    assert tuple(decode(0x532F85, 0x532F93).values()) == (
        ("lea", "eax, [edx + edx*2]"), ("lea", "esi, [edx + eax*4]"),
        ("shl", "esi, 2"), ("jmp", "0x53334c"))
    vehicle = decode(0x532FFD, 0x533048)
    base = decode(0x5330A1, 0x5330C0)
    assert vehicle[0x53302C] == ("mov", "esi, dword ptr [0x9a64c8]")
    assert vehicle[0x533032] == ("test", "esi, esi")
    assert vehicle[0x533034] == ("je", "0x534120")
    assert tuple(vehicle[a] for a in (0x53303A, 0x53303D, 0x533040)) == (
        ("lea", "ecx, [esi + esi*2]"), ("lea", "esi, [esi + ecx*4]"), ("shl", "esi, 2"))
    assert vehicle[0x533018][0] == vehicle[0x53301D][0] == "call"
    assert vehicle[0x533022] == ("mov", "dword ptr [edi + 0x1b48], 5")
    assert base[0x5330A1] == ("mov", "esi, dword ptr [0x9a64cc]")
    assert base[0x5330A7] == ("test", "esi, esi")
    assert base[0x5330A9] == ("je", "0x534120")
    assert tuple(base[a] for a in (0x5330AF, 0x5330B2, 0x5330B5, 0x5330B8)) == (
        ("lea", "ecx, [esi + esi*8]"), ("lea", "edx, [esi + ecx*2]"),
        ("lea", "esi, [esi + edx*4]"), ("shl", "esi, 2"))
    assert tuple(decode(0x533117, 0x533125).values()) == (
        ("mov", "esi, dword ptr [0x9a64d4]"), ("shl", "esi, 4"), ("jmp", "0x53334c"))
    assert tuple(decode(0x533166, 0x533171).values()) == (
        ("mov", "esi, dword ptr [0x9469f8]"), ("jmp", "0x53334c"))

    common = decode(0x53334C, 0x533387)
    assert tuple(common[a] for a in (0x53334C, 0x53334E, 0x53334F, 0x533355,
                                     0x533357, 0x53335F, 0x533362)) == (
        ("mov", "eax, esi"), ("cdq", ""), ("and", "edx, 0x7ff"),
        ("add", "eax, edx"), ("sar", "eax, 0xb"), ("shl", "edx, 0xb"),
        ("sub", "esi, edx"))
    assert common[0x533367] == ("je", "0x53336d")
    assert common[0x533369] == ("inc", "eax")
    assert common[0x533376] == ("jle", "0x533f95")
    receive = decode(0x53339A, 0x5333DD)
    assert receive[0x5333A0] == ("movsx", "ecx, word ptr [ecx + 0x12]")
    assert receive[0x5333A4] == ("cmp", "ecx, edi")
    assert receive[0x5333AB] == ("test", "esi, esi")
    assert receive[0x5333B9] == ("mov", "dword ptr [ebp + 0xc], eax")
    assert receive[0x5333D3] == ("mov", "dword ptr [ebp + 0xc], 0x800")
    assert receive[0x5333DA] == ("shl", "esi, 0xb")
    send = decode(0x533400, 0x533431)
    assert send[0x533407] == ("test", "edi, edi")
    assert send[0x533409] == ("je", "0x533427")
    assert send[0x533412] == ("mov", "dword ptr [ebp + 0xc], edi")
    assert send[0x533419] == ("mov", "dword ptr [ebp + 0xc], edi")
    packet = decode(0x533A82, 0x533AF5)
    assert packet[0x533A85] == ("lea", "eax, [edx + 0x24]")
    assert packet[0x533A8C] == ("call", "0x5d4510")
    assert packet[0x533ADD] == ("lea", "eax, [edi + 0x24]")
    assert packet[0x533AE0] == ("push", "edx")
    assert packet[0x533AE3] == ("call", "0x645930")
    # Only observation hooks execute malformed lengths in host controls.
    # No assertion here establishes allocation success, copy faults or neighbors.
