"""Original runtime initialization evidence, not replacement CRT definitions."""

from pathlib import Path

import pytest

from match_recovery.component_commands import execute
from recovery.environment.wine import VC6_ROOT


def test_native_original_runtime_initializers_keep_unproved_common_storage_visible():
    root = Path(__file__).resolve().parents[2]
    image = root / ".opensmacx/game/terranx_original.exe"
    archive = VC6_ROOT / "LIB/LIBCMT.LIB"
    if not image.is_file() or not archive.is_file():
        pytest.skip("local original image and compiler runtime are required")
    status, report = execute(root, {"action": "check-binary", "component": "vc6-runtime",
                                    "image": image, "archive": archive})
    assert status == 3 and report["result"] == "unknown", report
    rows = {row["symbol"]: row for row in report["functions"]}
    for symbol, address in (("__heap_init", "0x00647D7A"), ("___sbh_heap_init", "0x00647DB6")):
        row = rows[symbol]
        assert row["address"] == address and row["boundary"] == "pass", row
        assert row["result"] == "unknown", row
        assert any(item["symbol"] == "__crtheap" and item["result"] == "unknown"
                   for item in row["relocations"])
    imports = {row["symbol"]: row for row in report["import_bindings"]}
    for symbol, name in (("__imp__HeapCreate@12", "HeapCreate"),
                         ("__imp__HeapDestroy@4", "HeapDestroy"),
                         ("__imp__HeapAlloc@12", "HeapAlloc")):
        assert imports[symbol]["identity"] == "pass"
        assert imports[symbol]["module"] == "KERNEL32.dll" and imports[symbol]["name"] == name
    assert "__crtheap" in report["unresolved_symbols"]
    assert "___sbh_sizeHeaderList" in report["unresolved_symbols"]
    assert report["application_recoveries"] == 0 and not report["ready_for_collection"]
