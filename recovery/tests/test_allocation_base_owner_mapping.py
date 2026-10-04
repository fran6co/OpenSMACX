"""Source identity continuity; native/lifetime proof is checked separately."""
from pathlib import Path

from match_recovery.source import read_file
from match_recovery.source.model import Mode
from match_recovery.source_integrity import check_annotation_text
from match_recovery.bound_objects import FunctionBinding, check_objects
from match_recovery.model import Origin, Span
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin
from test_stringlist_recovery import objects


ROOT = Path(__file__).resolve().parents[2]


def test_allocation_publication_identity_belongs_to_genuine_base_destructor():
    path = ROOT / "src/stringstruct.cpp"
    record, = (r for r in read_file(path) if r.address == 0x402310)
    assert record.name == "sub_402310"
    assert record.image_spans == ((0x402310, 0x40231F),)
    assert record.symbol == "??1StringAllocationBase@@UAE@XZ"
    assert record.mode is Mode.BODY and record.byte_exact
    assert not record.extract_error
    assert "StringAllocationBase::~StringAllocationBase()" in record.region
    assert "StringAllocationHeap = allocation_owner_;" in record.region
    assert not (ROOT / "src/recovered/00402310.cpp").exists()
    assert not check_annotation_text(path, path.read_text())["findings"]


def test_real_allocation_base_destructor_matches_its_table_and_heap_targets(objects):
    pin = image_pin(ROOT)
    image = PeImage((ROOT / ".opensmacx/game/terranx_original.exe").read_bytes(), pin)
    record, = (r for r in read_file(ROOT / "src/stringstruct.cpp") if r.address == 0x402310)
    binding = FunctionBinding(Origin(pin, "pe:va", record.address),
                              tuple(Span(a, b) for a, b in record.image_spans),
                              record.symbol, "src/stringstruct.cpp")
    targets = {"??_7StringAllocationBase@@6B@": 0x6693AC,
               "?StringAllocationHeap@@3PAVHeap@@A": 0x9B3374}
    for obj in objects["src/stringstruct.cpp"]:
        row, = check_objects([binding], {"src/stringstruct.cpp": obj}, image,
                             external_targets=targets)
        assert row["boundary"] == row["bytes"] == row["references"] == "pass", row
        damaged, = check_objects([binding], {"src/stringstruct.cpp": obj}, image,
                                 external_targets=dict(targets, **{
                                     "?StringAllocationHeap@@3PAVHeap@@A": 0x9B3378}))
        assert damaged["references"] == "fail", damaged
