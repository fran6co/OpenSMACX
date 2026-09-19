"""Fresh physical PE/PDB controls through the actual allocation consumer."""

import struct

import pytest

from match_recovery.linked_context import linked_build
import test_ordinary_allocation_coverage as ordinary
from test_linked_initializers import update_artifact
from test_linked_modules import set_modules
from test_pdb_contributions import contribution, module


@pytest.mark.parametrize("damage", [None, "missing_fixup", "wrong_fixup", "duplicate_contribution"])
def test_generated_storage_cannot_close_damaged_allocation_domain(tmp_path, monkeypatch, damage):
    original = ordinary.set_contributions

    def construct(context, records):
        # Build a new fixture before it seals its native epoch. No verdicts or
        # guards are replaced; only test-owned physical PE and PDB bytes change.
        image = bytearray((context.directory / "app.exe").read_bytes())
        image[0x280:0x286] = b"\xff\x25" + struct.pack("<I", 0x402150)
        struct.pack_into("<II", image, 0x98 + 104, 0x2100, 40)
        struct.pack_into("<IIIII", image, 0x500, 0x2140, 0, 0, 0x2160, 0x2150)
        struct.pack_into("<II", image, 0x540, 0x2170, 0)
        struct.pack_into("<II", image, 0x550, 0x2170, 0)
        image[0x560:0x569] = b"DEMO.dll\0"
        image[0x570:0x578] = b"\2\0Paint\0"
        site = 0 if damage == "missing_fixup" else 0x3081 if damage == "wrong_fixup" else 0x3082
        struct.pack_into("<IIHH", image, 0x380, 0x1000, 12, site, 0)
        struct.pack_into("<II", image, 0x98 + 96 + 5 * 8, 0x1180, 12)
        update_artifact(context, "app.exe", bytes(image))
        label = str(context.objects[0]).encode()
        set_modules(context, module(label, label) + module(b"* Linker *", b""))
        thunk = contribution(section=1, offset=0x80, size=6, flags=0x60000020, module=1)
        original(context, records + thunk * (2 if damage == "duplicate_contribution" else 1))

    monkeypatch.setattr(ordinary, "set_contributions", construct)
    with linked_build(ordinary.fixture(tmp_path / "project")) as live:
        report = live.ordinary_allocations()
        if damage is None:
            assert report["result"] == "pass", report
            assert report["generated_storage"]["assigned"] == 1
        else:
            assert report["allocation_domain"] != "pass", {
                key: report[key] for key in ("result", "allocation_domain", "allocation_isolation",
                                            "unmatched_contributions", "gaps")}
            assert report["generated_storage"]["assigned"] == 0, report["generated_storage"]
            assert report["allocation_isolation"] != "pass", report
            assert report["unmatched_contributions"], report
        assert report["remaining_linked_effects"] == "unknown"
        assert not report["ready_for_collection"]
