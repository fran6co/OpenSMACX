"""Source identity continuity; native/lifetime proof is checked separately."""
from pathlib import Path
import os

from match_recovery.source_integrity import check_annotation_text
from match_recovery.source.model import Mode
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from test_stringlist_recovery import objects
from test_pact_ascent import definition
from owner_transfer_test_support import current_record_by_address


ROOT = Path(__file__).resolve().parents[2]


def test_allocation_publication_identity_belongs_to_genuine_base_destructor():
    record = current_record_by_address(ROOT, 0x402310)
    path = record.path
    header = ROOT / "src/stringstruct.h"
    assert record.name == "sub_402310"
    assert record.image_spans == ((0x402310, 0x40231F),)
    assert record.symbol == "??1StringAllocationBase@@UAE@XZ"
    assert record.mode is Mode.FILE and record.byte_exact
    assert record.body == "src/stringstruct.h"
    assert record.context == "src/alphanet.cpp"
    assert not record.extract_error
    destructor = definition(header.read_text(encoding="utf-8"),
                            r"virtual\s+~StringAllocationBase\s*\(\)")
    assert "StringAllocationHeap = allocation_owner_;" in destructor
    assert not (ROOT / "src/recovered/00402310.cpp").exists()
    assert not check_annotation_text(path, path.read_text())["findings"]


def test_real_allocation_base_destructor_matches_its_table_and_heap_targets(objects):
    pin = image_pin(ROOT)
    image = PeImage(Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe")).read_bytes(), pin)
    record = current_record_by_address(ROOT, 0x402310)
    physical_unit = record.context
    binding = FunctionBinding(Origin(pin, "pe:va", record.address),
                              tuple(Span(a, b) for a, b in record.image_spans),
                              record.symbol, physical_unit)
    targets = {"??_7StringAllocationBase@@6B@": 0x6693AC,
               "?StringAllocationHeap@@3PAVHeap@@A": 0x9B3374}
    for obj in objects[physical_unit]:
        row, = check_objects([binding], {physical_unit: obj}, image,
                             external_targets=targets)
        assert row["boundary"] == row["bytes"] == row["references"] == "pass", row
        damaged, = check_objects([binding], {physical_unit: obj}, image,
                                 external_targets=dict(targets, **{
                                     "?StringAllocationHeap@@3PAVHeap@@A": 0x9B3378}))
        assert damaged["references"] == "fail", damaged
