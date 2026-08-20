"""What one function calls, in the order the image calls it."""

import os
from pathlib import Path

import pytest

from decomp.asm import Listing
from decomp.calls import call_sites, imported_names

BASE = 0x00401000

# call 0x402000 / call [0x9B8178] / call eax / jmp +0 / jmp 0x402000 / ret
BODY = bytes.fromhex(
    "E8FB0F0000"      # 0x401000  direct, to 0x402000
    "FF1578819B00"    # 0x401005  indirect through a slot
    "FFD0"            # 0x40100B  dynamic, through a register
    "EB00"            # 0x40100D  jmp INSIDE - control flow, not an edge
    "E9EC0F0000"      # 0x40100F  jmp OUTSIDE - a tail call
    "C3")             # 0x401014


def sites():
    return call_sites(Listing(code=BODY, base=BASE))


def test_every_form_is_recognised():
    forms = [s.form for s in sites()]
    assert forms == ["direct", "indirect", "dynamic", "tail"]


def test_the_order_is_the_image_s_order():
    """The annotation's `calls` fact is a sorted SET, so it cannot say what
    happens first, and cannot say a target is reached twice. That is the
    whole reason to read the bytes."""
    assert [s.at for s in sites()] == sorted(s.at for s in sites())
    assert sites()[0].at == 0x00401000
    assert sites()[-1].at == 0x0040100F


def test_a_jump_inside_the_body_is_not_an_edge():
    """Otherwise every loop reads as a self-call."""
    assert all(s.at != 0x0040100D for s in sites())


def test_a_direct_call_resolves_and_an_indirect_one_names_its_slot():
    direct, indirect, dynamic, tail = sites()
    assert direct.target == 0x00402000 and direct.slot is None
    assert indirect.target is None and indirect.slot == 0x009B8178
    assert dynamic.target is None and dynamic.slot is None
    assert tail.target == 0x00402000


def test_a_repeated_target_is_repeated():
    """`world_climate` calls `log_say` and `do_checksums` twice each; a set
    reports one edge where the image makes two."""
    twice = Listing(code=b"\xE8\xFB\x0F\x00\x00\xE8\xF6\x0F\x00\x00\xC3",
                    base=BASE)
    assert [s.target for s in call_sites(twice)] == [0x00402000, 0x00402000]


def test_the_code_data_boundary_is_respected():
    """A jump table past the function is data - decoding it would invent
    edges out of address bytes. See `Listing.code_bytes`."""
    padded = Listing(code=BODY + b"\xE8\x00\x00\x00\x00", base=BASE,
                     code_bytes=len(BODY))
    assert len(call_sites(padded)) == len(sites())


# ------------------------------------------------------- against the image

EXE = Path(__file__).resolve().parents[2] / ".opensmacx/game/terranx_original.exe"


@pytest.mark.skipif(not EXE.is_file(), reason=f"no image at {EXE}")
def test_the_import_table_names_the_slots_the_code_calls_through():
    """An `FF 15` through one of these leaves the program, so it is never a
    recovery target - and saying which one it leaves through is most of what
    a reader wants from an indirect call."""
    imports = imported_names(EXE)
    assert imports[0x00669058] == "GDI32.dll!DeleteObject"
    assert imports[0x0066927C] == "USER32.dll!GetDC"
    assert imports[0x00669368] == "WINMM.dll!timeGetTime"
    assert len(imports) > 200


@pytest.mark.skipif(not EXE.is_file(), reason=f"no image at {EXE}")
def test_a_file_that_is_not_a_pe_says_so(tmp_path):
    not_pe = tmp_path / "x.bin"
    not_pe.write_bytes(b"\x00" * 0x200)
    with pytest.raises(ValueError, match="not a PE"):
        imported_names(not_pe)
