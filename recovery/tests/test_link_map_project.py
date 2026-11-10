"""The project's own VC6 map parses and validates against its linked image."""
import hashlib
from pathlib import Path

import pytest

from match_recovery.msvc_link_map import parse_link_map
from match_recovery.pe_coff import PeImage

BUILD = Path(__file__).resolve().parents[2] / "build"
MAP, IMAGE = BUILD / "OpenSMACX.map", BUILD / "OpenSMACX.exe"


@pytest.mark.skipif(not (MAP.is_file() and IMAGE.is_file()), reason="no linked project build")
def test_project_map_retains_optional_tables_and_validates_coordinates():
    mapping = parse_link_map(MAP.read_bytes())
    raw = IMAGE.read_bytes()
    mapping.validate(PeImage(raw, hashlib.sha256(raw).hexdigest()))
    assert mapping.contributions and mapping.statics
    assert any(item.name == ".CRT$XCU" for item in mapping.contributions)
    # VC6 names every TU's dynamic initializer _$E19: one static per owner.
    owners = {item.owner for item in mapping.statics if item.symbol == "_$E19"}
    assert len(owners) > 1
