"""Bounded controls compile the actual movement helpers, not a copied recovery."""
from pathlib import Path
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
FIXTURES = Path(__file__).resolve().parent / "fixtures"

def test_actual_vehicle_order_helpers_gcc_and_clang():
    source = (ROOT / "src/veh.cpp").read_text()
    helpers = source.split("// Vehicle movement owner helpers.\n", 1)[1].split("// Vehicle movement owner entry.\n", 1)[0]
    fixture = (FIXTURES / "order_veh_tail_control.cpp").read_text()
    assert fixture.count("/* PRODUCT_HELPERS */") == 1
    unit = fixture.replace("/* PRODUCT_HELPERS */", helpers)
    available = [path for name in ("g++", "clang++") if (path := shutil.which(name))]
    assert available, "a host C++ compiler is required"
    with tempfile.TemporaryDirectory() as directory:
        temporary = Path(directory)
        cpp = temporary / "control.cpp"
        cpp.write_text(unit)
        for index, compiler in enumerate(available):
            executable = temporary / ("control" + str(index))
            subprocess.run([compiler, "-std=c++98", "-ffunction-sections", "-fdata-sections", "-I", str(FIXTURES), str(cpp), "-Wl,--gc-sections", "-o", str(executable)], check=True, capture_output=True, text=True, timeout=60)
            subprocess.run([str(executable)], check=True, capture_output=True, text=True, timeout=30)
