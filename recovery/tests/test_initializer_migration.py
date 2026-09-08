"""Real source/image damage control for the migrated initializer gate."""

import hashlib
import json
from pathlib import Path

import pytest

from match_recovery.initializer_check import check_initializers


ROOT = Path(__file__).resolve().parents[2]


def test_missing_selected_base_initializer_is_detected_without_editing_product(tmp_path):
    image = ROOT / ".opensmacx/game/terranx_original.exe"
    if not image.is_file():
        pytest.skip("pinned original image is required for the real-project damage control")
    original = (ROOT / "src/base.cpp").read_bytes()
    marker = b"int BaseIDCurrentSelected = -1;  // 0x00689370"
    assert original.count(marker) == 1
    config = json.loads((ROOT / "recovery/project.json").read_text())
    (tmp_path / "src").mkdir()
    (tmp_path / "recovery").mkdir()
    (tmp_path / "src/IMAGE_PIN").write_bytes((ROOT / "src/IMAGE_PIN").read_bytes())
    (tmp_path / "recovery/project.json").write_text(json.dumps({
        "schema_version": 1, "source": config["source"], "initializers": config["initializers"]}))
    source = tmp_path / "src/base.cpp"
    source.write_bytes(original)
    before = check_initializers(tmp_path, image)
    assert before["result"] == "pass" and before["counts"]["checked"] > 0
    source.write_bytes(original.replace(marker, b"int BaseIDCurrentSelected;  // 0x00689370"))
    damaged = check_initializers(tmp_path, image)
    assert damaged["result"] == "fail"
    assert any(row["symbol"] == "BaseIDCurrentSelected" for row in damaged["findings"])
    source.write_bytes(original)
    assert check_initializers(tmp_path, image)["result"] == "pass"
    assert hashlib.sha256((ROOT / "src/base.cpp").read_bytes()).digest() == hashlib.sha256(original).digest()
