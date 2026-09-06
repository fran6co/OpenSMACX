"""Share synthetic binary fixtures without shipping project tests in the library."""

from pathlib import Path
import json
import sys
import pytest

sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "packages/match-recovery/tests"))
sys.path.insert(0, str(Path(__file__).resolve().parents[2] / "packages/match-recovery/tests/source"))


@pytest.fixture
def source_configuration(tmp_path):
    """Explicit source-provider inputs for synthetic application fixtures."""
    configuration = tmp_path / "recovery/project.json"
    configuration.parent.mkdir(exist_ok=True)
    value = {"schema_version": 1, "source": {
        "image_pin": "src/IMAGE_PIN", "address_space": "pe:va",
        "seed_roots": ["src/recovered", "src/unrecovered"], "symbol_format": "msvc"}}
    value["measurement"] = {
        "backend": "pe-coff-i386", "compiler_profile": "application",
        "flag_sets": [["/c", "/O2", "/Gy", "/GR-", "/GX"], ["/c", "/O2", "/Gy", "/GR-", "/Oy-", "/GX"]],
        "layout_flags": ["/Z7"], "unit_extensions": [".cpp", ".c"],
        "inputs": ["build/compile_commands.json", "recovery/environment/compiler.py"], "input_roots": []}
    configuration.write_text(json.dumps(value))
