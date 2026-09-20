"""Shared caption storage, not full MainInterface/runtime acceptance."""

import os
from pathlib import Path
import re
import shutil
import subprocess

import pytest
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

from match_recovery.model import Span
from match_recovery.object_source import read_objects
from match_recovery.pe_coff import PeImage
from match_recovery.source_inventory import image_pin


ROOT = Path(__file__).resolve().parents[2]


def caption_body():
    source = (ROOT / "src/mapwin.cpp").read_text()
    return re.search(r"void MapWin::main_caption\(\)\s*\{[^}]*\}", source).group()


def run_caption(tmp_path, body):
    compiler = shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    # Borrow the real owning definition: a duplicate caption allocation can
    # have the same spelling/extent yet cannot carry the string producer's data.
    strings = (ROOT / "src/strings.cpp").read_text()
    storage = re.search(r"char StringTemp\[\d+\];", strings).group()
    source = r'''
        #include <cstring>
        #include <cstdlib>
        STORAGE
        char decoy[1032];
        struct MainInterface { void set_date(char *); } MainInterfaceGlobal, wrong;
        struct MapWin { void main_caption(); } caller;
        const char *expected;
        int calls;
        void require(bool ok) { if (!ok) std::exit(41); }
        void MainInterface::set_date(char *text) {
            require(this == &MainInterfaceGlobal && text == StringTemp);
            require(std::strcmp(text, expected) == 0);
            ++calls;
            text[0] = '!';
        }
        BODY
        int main() {
            std::strcpy(decoy, "stale caption");
            const char *values[] = {"M.Y. 2100", "M.Y. 2197", ""};
            for (int i = 0; i < 3; ++i) {
                std::strcpy(StringTemp, values[i]);
                expected = values[i];
                caller.main_caption();
                require(StringTemp[0] == '!');
                require(std::strcmp(decoy, "stale caption") == 0);
            }
            require(calls == 3);
        }
    '''.replace("STORAGE", storage).replace("BODY", body)
    path = tmp_path / "caption.cpp"
    executable = tmp_path / "caption"
    path.write_text(source)
    compile_result = subprocess.run([compiler, "-std=c++11", "-O1", "-fsanitize=undefined",
                                    str(path), "-o", str(executable)], capture_output=True,
                                   text=True, timeout=30)
    assert compile_result.returncode == 0, compile_result.stderr
    return subprocess.run([str(executable)], capture_output=True, text=True, timeout=10)


def test_caption_borrows_live_string_storage(tmp_path):
    result = run_caption(tmp_path, caption_body())
    assert result.returncode == 0, result.stderr


@pytest.mark.parametrize("before,after", [
    ("set_date(StringTemp)", "set_date(decoy)"),
    ("MainInterfaceGlobal.set_date", "wrong.set_date"),
    ("set_date(StringTemp)", "set_date(StringTemp + 1)"),
])
def test_caption_rejects_wrong_owner_or_offset(tmp_path, before, after):
    body = caption_body()
    assert body.count(before) == 1
    result = run_caption(tmp_path, body.replace(before, after))
    assert result.returncode == 41, result.stderr


def test_original_caption_uses_the_existing_scratch_owner():
    path = Path(os.environ.get("OPENSMACX_IMAGE", ROOT / ".opensmacx/game/terranx_original.exe"))
    if not path.is_file():
        pytest.skip("pinned original required")
    pin = image_pin(ROOT)
    image = PeImage(path.read_bytes(), pin)
    ins = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm(image.read(Span(0x46FB10, 0x46FB20)), 0x46FB10))
    assert [(i.mnemonic, i.op_str) for i in ins] == [
        ("push", "0x9b86a0"), ("mov", "ecx, 0x7ae820"),
        ("call", "0x45be80"), ("ret", ""),
    ]
    owner, = (o for o in read_objects(ROOT, pin) if o.origin.address == 0x9B86A0)
    assert owner.unit == "src/strings.cpp" and owner.symbol == "?StringTemp@@3PADA"
