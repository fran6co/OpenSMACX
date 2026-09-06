"""Original import-table bindings are project integration evidence."""
from pathlib import Path
import os
import pytest
from match_recovery.source.calls import imported_names

EXE = Path(os.environ.get("OPENSMACX_IMAGE", str(Path(__file__).resolve().parents[2] / ".opensmacx/game/terranx_original.exe")))

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
