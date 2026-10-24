"""The AlphaNet session records remain one genuine source owner after the split."""

from pathlib import Path
import re

from match_recovery.source import Mode, read


ROOT = Path(__file__).resolve().parents[2]
RECORDS = {
    0x401C80, 0x402B40, 0x4E3300, 0x4E3350, 0x4E34B0,
    0x5D4890, 0x5D4990, 0x401D80, 0x401CE0, 0x5D49D0, 0x5D4BB0,
}
ALPHANET_ROWS = {
    0x401C80, 0x402B40, 0x4E2400, 0x4E2460, 0x4E29E0, 0x4E2A30,
    0x4E2E50, 0x4E31E0, 0x4E3300, 0x4E3350, 0x4E34B0,
    0x4E25E0, 0x4E2610, 0x4E2660, 0x4E26B0, 0x4E2490,
    0x538B60, 0x4E32F0, 0x4E32C0, 0x4E32D0, 0x4E32E0,
    0x5D4890, 0x5D4990, 0x4E25B0, 0x401D80, 0x401CE0,
    0x5D49D0, 0x5D4BB0, 0x4E26E0,
}


def test_complete_alpha_identity_set_has_one_real_owner_per_mapping():
    records_path = ROOT / "src/alphanet_records.cpp"
    controller_path = ROOT / "src/alphanet.cpp"
    rows = [row for row in read(ROOT / "src") if row.address in ALPHANET_ROWS]
    assert len(rows) == len(ALPHANET_ROWS)
    by_address = {row.address: row for row in rows}
    assert set(by_address) == ALPHANET_ROWS
    for address in (0x401CE0, 0x4E3300, 0x4E3350, 0x4E34B0):
        assert by_address[address].context == "src/alphanet.cpp"
    for address in (0x401CE0, 0x4E3350):
        assert by_address[address].mode is Mode.FILE
        assert by_address[address].body == "src/alphanet.h"
    assert not by_address[0x402B40].context
    assert {row.address for row in rows if row.path == records_path.resolve()} == RECORDS
    assert {row.address for row in rows if row.path == controller_path.resolve()} == ALPHANET_ROWS - RECORDS
    assert len({row.address for row in rows if row.path not in {
        records_path.resolve(), controller_path.resolve()}}) == 0


def test_records_and_controller_code_stay_in_their_coherent_units():
    records = (ROOT / "src/alphanet_records.cpp").read_text()
    controller = (ROOT / "src/alphanet.cpp").read_text()
    header = (ROOT / "src/alphanet.h").read_text()
    for signature in (
        "AlphaNetStringStruct::AlphaNetStringStruct()",
        "AlphaNetStringStruct::add(int id)",
        "AlphaNetStringStruct::sort_sessions()",
        "AlphaNetLedger::on_insert(NetAllocationNode *payload)",
        "AlphaNetLedger::on_remove(NetAllocationNode *payload)",
        "compare_session_guid(const GUID *left, const GUID *right)",
    ):
        assert signature in records, signature
    for name in ("AlphaNetStringStruct", "AlphaNetLedger"):
        assert f"{name}::~{name}()" not in records
        body = re.search(rf"~{name}\(\)\s*\{{([^{{}}]*)\}}", header)
        assert body is not None
        assert body.group(1).split() == ["remove_all();", "current_position_", "=", "0;"]
    assert "AlphaNetStringStruct::add(int id)" not in controller
    assert "AlphaNetLedger::on_insert" not in controller
    assert "AlphaNet::close()" in controller
    assert "reset_net_session_options()" in controller
    assert "reset_net_lobby_rows()" in controller


def test_record_owner_uses_default_profile_and_controller_keeps_measured_frames():
    cmake = (ROOT / "CMakeLists.txt").read_text()
    sources = re.search(r"set\(OPENSMACX_SOURCES(.*?)\n\)", cmake, re.S)
    assert sources is not None
    source_rows = re.findall(r"src/[A-Za-z0-9_./-]+\.cpp", sources.group(1))
    index = source_rows.index("src/alphanet.cpp")
    assert source_rows[index + 1] == "src/alphanet_records.cpp"
    assert source_rows[index + 2] == "src/ambience.cpp"
    profiles = re.findall(r"set_source_files_properties\((.*?)\)", cmake, re.S)
    alpha_profile = [body for body in profiles if "src/alphanet.cpp" in body]
    assert len(alpha_profile) == 1
    assert 'COMPILE_OPTIONS "/Oy-"' in alpha_profile[0]
    assert not any("src/alphanet_records.cpp" in body for body in profiles)
